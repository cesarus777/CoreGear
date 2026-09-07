#!/usr/bin/env bash

# Project workflow entry point. Typical invocations are:
#   ./orch.sh lint [--inplace]
#   ./orch.sh everything --preset base_with_tests --profile gcc.txt
#   ./orch.sh config|build|test [subcommand options]
# `everything` synchronizes development tools, installs Conan dependencies,
# configures, builds, and tests in that order.

set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

readonly this_script_path="$0"

# Echo a shell-escaped representation for diagnostics, then execute the
# argument vector directly. This deliberately avoids constructing `sh -c`
# command strings from user-supplied options.
run() {
	printf '+ '
	printf '%q ' "$@"
	printf '\n'
	"$@"
}

# Build directories and Conan profiles become tool arguments and determine
# where generated files are written. Keep them relative to this checkout and
# reject traversal or multi-line values before invoking external tools.
require_safe_path() {
	case "$1" in
	"" | /* | *".."* | *$'\n'* | *$'\r'*)
		echo "Unsafe path '$1'" >&2
		exit 2
		;;
	esac
}

usage() {
	echo "[USAGE]: ${this_script_path} {everything|uv|config|build|test} [subcommand specific options]"
}

run_uv() {
	local exit_at_end=""
	local build_type="Release"
	local build_dir="build/$build_type"
	local conan_profile="conan/gcc.txt"
	local uv_command=""

	while [ $# -gt 0 ]; do
		case "$1" in
		"__exit_at_end")
			exit_at_end="yes"
			shift
			;;
		"--profile" | "-p")
			conan_profile="$2"
			shift 2
			;;
		"--build-dir" | "-b")
			build_dir="$2"
			shift 2
			;;
		"--conan-install" | "-i")
			if [[ -n "${uv_command}" ]]; then
				echo "You can pass only one uv wrapper command at the same time" >&2
				exit 2
			fi
			uv_command="conan-install"
			shift
			;;
		"--sync" | "-s")
			if [[ -n "${uv_command}" ]]; then
				echo "You can pass only one uv wrapper command at the same time" >&2
				exit 2
			fi
			uv_command="sync"
			shift
			;;
		*)
			echo "Unknown option '$1'" >&2
			usage
			exit 2
			;;
		esac
	done

	# The same profile applies to Conan's host and build contexts (`-pr:a`).
	# This lets its platform tool requirements, including Nix-provided CMake,
	# replace matching Conan tool packages in either context.
	require_safe_path "${build_dir}"
	require_safe_path "${conan_profile}"

	if ! command -v uv >/dev/null 2>&1; then
		echo "uv is required; install it using your trusted system package workflow." >&2
		exit 127
	fi

	case "${uv_command}" in
	"conan-install")
		# Conan writes a CMake toolchain into the requested build directory.
		# Do not let it generate a user preset: this repository owns its committed
		# presets and passes the generated toolchain explicitly during configure.
		run uv run conan install . -of "${build_dir}" -pr:a "${conan_profile}" --build=missing -c tools.cmake.cmaketoolchain:user_presets=
		;;
	"sync")
		run uv sync --locked
		;;
	"")
		echo "A uv wrapper command is required" >&2
		exit 2
		;;
	esac
	if [ "$exit_at_end" = "yes" ]; then exit 0; fi
}

run_lint() {
	local exit_at_end=""
	local inplace=""

	while [ $# -gt 0 ]; do
		case "$1" in
		"__exit_at_end")
			exit_at_end="yes"
			shift
			;;
		"--inplace" | "-i")
			inplace="-i"
			shift 1
			;;
		*)
			echo "Unknown option '$1'" >&2
			usage
			exit 2
			;;
		esac
	done

	# NUL-delimited discovery preserves arbitrary filenames and avoids invoking
	# a formatter with no inputs. C++ source is intentionally limited to coregear.
	mapfile -d '' -t cpp_files < <(find coregear -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.mpp' \) -print0)
	if ((${#cpp_files[@]})); then
		if [[ -n "${inplace}" ]]; then
			run uv run clang-format -i "${cpp_files[@]}"
		else
			run uv run clang-format --Werror -n "${cpp_files[@]}"
		fi
	fi

	# CMake files can exist at any repository level. Exclude generated and
	# metadata directories so lint results depend only on tracked source files.
	mapfile -d '' -t cmake_files < <(find . \( -name .cache -o -name build -o -name .git \) -type d -prune -o -type f -name CMakeLists.txt -print0)
	if [[ -n "${inplace}" ]]; then
		run uv run cmake-format -i "${cmake_files[@]}"
		run uv run cmake-lint --suppress-decorations "${cmake_files[@]}"
	else
		run uv run cmake-format --check "${cmake_files[@]}"
		run uv run cmake-lint "${cmake_files[@]}"
	fi

	if [[ -n "${inplace}" ]]; then
		run uv run shfmt -w "${this_script_path}"
	else
		run uv run shfmt -d "${this_script_path}"
	fi

	run uv run shellcheck "${this_script_path}"

	if [[ -n "${inplace}" ]]; then
		run uv run typos -w
	else
		run uv run typos
	fi

	if [ "$exit_at_end" = "yes" ]; then exit 0; fi
}

run_configure() {
	local exit_at_end=""
	local preset="base"
	local build_type="Release"
	local build_dir="build/$build_type"

	while [ $# -gt 0 ]; do
		case "$1" in
		"__exit_at_end")
			exit_at_end="yes"
			shift
			;;
		"--preset" | "-p")
			preset="$2"
			shift 2
			;;
		"--build-type")
			build_type="$2"
			shift 2
			;;
		"--build-dir" | "-b")
			build_dir="$2"
			shift 2
			;;
		*)
			echo "Unknown option '$1'" >&2
			usage
			exit 2
			;;
		esac
	done

	# `run_uv --conan-install` must run first: it generates this toolchain file.
	require_safe_path "${build_dir}"
	run cmake -S . -B "${build_dir}" "-DCMAKE_BUILD_TYPE=${build_type}" --preset "${preset}" "--toolchain=${build_dir}/conan_toolchain.cmake"
	if [ "$exit_at_end" = "yes" ]; then exit 0; fi
}

run_build() {
	local exit_at_end=""
	local target="all"
	local build_type="Release"
	local build_dir="build/$build_type"

	while [ $# -gt 0 ]; do
		case "$1" in
		"__exit_at_end")
			exit_at_end="yes"
			shift
			;;
		"--target" | "-t")
			target="$2"
			shift 2
			;;
		"--build-dir" | "-b")
			build_dir="$2"
			shift 2
			;;
		*)
			echo "Unknown option '$1'" >&2
			usage
			exit 2
			;;
		esac
	done

	require_safe_path "${build_dir}"
	run cmake --build "${build_dir}" --target "${target}"
	if [ "$exit_at_end" = "yes" ]; then exit 0; fi
}

run_test() {
	local exit_at_end=""
	local build_type="Release"
	local build_dir="build/$build_type"
	local lint=""

	while [ $# -gt 0 ]; do
		case "$1" in
		"__exit_at_end")
			exit_at_end="yes"
			shift
			;;
		"--build-dir" | "-b")
			build_dir="$2"
			shift 2
			;;
		"--lint" | "-l")
			lint="yes"
			shift
			;;
		*)
			echo "Unknown option '$1'" >&2
			usage
			exit 2
			;;
		esac
	done

	require_safe_path "${build_dir}"
	run ctest --test-dir "${build_dir}"
	if [ "$lint" = "yes" ]; then run_lint; fi
	if [ "$exit_at_end" = "yes" ]; then exit 0; fi
}

run_everything() {
	local everything_preset="base"
	local everything_profile="gcc.txt"
	local everything_build_type="Release"
	local everything_build_dir="build/$everything_build_type"
	local everything_lint=""
	while [ $# -gt 0 ]; do
		case "$1" in
		"--preset" | "-p")
			everything_preset="$2"
			shift 2
			;;
		"--profile" | "-pr")
			everything_profile="$2"
			shift 2
			;;
		"--build-type")
			everything_build_type="$2"
			shift 2
			;;
		"--build-dir" | "-b")
			everything_build_dir="$2"
			shift 2
			;;
		"--lint" | "-l")
			everything_lint="--lint"
			shift
			;;
		*)
			echo "Unknown option '$1'" >&2
			usage
			exit 2
			;;
		esac
	done

	# Keep the tool environment and Conan graph reproducible before consuming
	# either. Each stage receives the same build directory selected by the user.
	run_uv --sync
	run_uv --conan-install --build-dir "${everything_build_dir}" --profile "conan/${everything_profile}"
	run_configure --preset "${everything_preset}" --build-type "${everything_build_type}" --build-dir "${everything_build_dir}"
	run_build --build-dir "${everything_build_dir}"
	run_test --build-dir "${everything_build_dir}" ${everything_lint}

	exit 0
}

main() {
	if [ $# -lt 1 ]; then
		echo "Subcommand expected" >&2
		exit 2
	fi

	# The internal sentinel makes individual subcommands exit after completion;
	# `everything` instead composes their functions in one shell process.
	local subcommand="$1"
	shift
	case "$subcommand" in
	"uv")
		run_uv "__exit_at_end" "$@"
		;;
	"lint")
		run_lint "__exit_at_end" "$@"
		;;
	"config")
		run_configure "__exit_at_end" "$@"
		;;
	"build")
		run_build "__exit_at_end" "$@"
		;;
	"test")
		run_test "__exit_at_end" "$@"
		;;
	"everything")
		run_everything "$@"
		;;
	"--help" | "-h")
		usage
		exit 0
		;;
	*)
		echo "Unknown subcommand '$subcommand'" >&2
		usage
		exit 2
		;;
	esac
}

main "$@"
