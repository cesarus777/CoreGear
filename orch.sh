#!/usr/bin/env bash

# Project workflow entry point. Typical invocations are:
#   ./orch.sh lint [--inplace]
#   ./orch.sh everything --preset base_with_tests
#   ./orch.sh config|build|test [subcommand options]
# `everything` configures, builds, and tests in that order.

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

# Build directories become tool arguments and determine where generated files
# are written. Keep them relative to this checkout and reject traversal or
# multi-line values before invoking external tools.
require_safe_path() {
	case "$1" in
	"" | /* | *".."* | *$'\n'* | *$'\r'*)
		echo "Unsafe path '$1'" >&2
		exit 2
		;;
	esac
}

usage() {
	echo "[USAGE]: ${this_script_path} {everything|lint|config|build|test} [subcommand specific options]"
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
			run clang-format -i "${cpp_files[@]}"
		else
			run clang-format --Werror -n "${cpp_files[@]}"
		fi
	fi

	# CMake files can exist at any repository level. Exclude generated and
	# metadata directories so lint results depend only on tracked source files.
	mapfile -d '' -t cmake_files < <(find . \( -name .cache -o -name build -o -name .git \) -type d -prune -o -type f -name CMakeLists.txt -print0)
	if [[ -n "${inplace}" ]]; then
		run cmake-format -i "${cmake_files[@]}"
		run cmake-lint --suppress-decorations "${cmake_files[@]}"
	else
		run cmake-format --check "${cmake_files[@]}"
		run cmake-lint "${cmake_files[@]}"
	fi

	if [[ -n "${inplace}" ]]; then
		run shfmt -w "${this_script_path}"
	else
		run shfmt -d "${this_script_path}"
	fi

	run shellcheck "${this_script_path}"

	if [[ -n "${inplace}" ]]; then
		run typos -w
	else
		run typos
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

	require_safe_path "${build_dir}"
	run cmake -S . -B "${build_dir}" "-DCMAKE_BUILD_TYPE=${build_type}" --preset "${preset}"
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
	local everything_build_type="Release"
	local everything_build_dir="build/$everything_build_type"
	local everything_lint=""
	while [ $# -gt 0 ]; do
		case "$1" in
		"--preset" | "-p")
			everything_preset="$2"
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

	# Nix supplies the tools and dependencies for each stage.
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
