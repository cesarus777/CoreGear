#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

usage() {
  echo "[USAGE]: $0 {config|build|test} [subcommand specific options]"
}

run_configure() {
  exit_at_end=""
  preset="base"
  build_type="Release"
  build_dir="build/$build_type"
  clang_stdlib_modules_wa="OFF"

  while [ $# -gt 0 ]; do
    case "$1" in
      "__exit_at_end")
        exit_at_end="yes"
        shift
        ;;
      "--clang-libcxx-wa")
        clang_stdlib_modules_wa="ON"
        shift 1
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

  sh -x -c "cmake -S . -B $build_dir -DCMAKE_BUILD_TYPE=$build_type --preset $preset -DCLANG_STDLIB_MODULES_JSON_WA=$clang_stdlib_modules_wa"
  if [ "$exit_at_end" = "yes" ]; then exit 0; fi }

run_build() {
  exit_at_end=""
  target="all"
  build_type="Release"
  build_dir="build/$build_type"

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

  sh -x -c "cmake --build $build_dir --target $target"
  if [ "$exit_at_end" = "yes" ]; then exit 0; fi
}

run_test() {
  exit_at_end=""
  build_type="Release"
  build_dir="build/$build_type"

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
      *)
        echo "Unknown option '$1'" >&2
        usage
        exit 2
        ;;
    esac
  done

  sh -x -c "ctest --test-dir $build_dir"
  if [ "$exit_at_end" = "yes" ]; then exit 0; fi
}

run_everything() {
  preset="base"
  build_type="Release"
  build_dir="build/$build_type"
  clang_libcxx_wa=""
  while [ $# -gt 0 ]; do
    case "$1" in
      "--clang-libcxx-wa")
        clang_libcxx_wa="--clang-libcxx-wa"
        shift 1
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

  run_configure --preset "$preset" --build-type "${build_type}" --build-dir "${build_dir}" $clang_libcxx_wa
  run_build --build-dir "${build_dir}"
  run_test --build-dir "${build_dir}"

  exit 0
}

main() {
  if [ $# -lt 1 ]; then
    echo "Subcommand expected" >&2
    exit 2
  fi

  subcommand="$1"
  shift
  case "$subcommand" in
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
