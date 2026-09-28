# Nix dependency migration

This document tracks the transition from Conan and uv to the pinned Nix flake.
It is separate from the ongoing machine-description implementation in
`docs/machine-description.md`.

## Acceptance criteria

- The locked Nixpkgs revision supplies the GCC 16 toolchain, ELFIO, GoogleTest,
  CMake, Ninja, the RISC-V cross compiler, and every lint tool.
- `nix develop` supports `./orch.sh lint` and
  `./orch.sh everything --preset base_with_tests --lint` without Conan or uv.
- `nix run .#lint` and `nix run .#check` work non-interactively; CMake discovers
  the Nix libraries in both workflows.
- Conan profiles and locks, the uv project and lock, and obsolete commands are
  removed. Repository and design guidance describe the new workflow.
- The focused configure, build, test, lint, and flake checks pass, or their
  exact blockers are recorded here.

## Completed migration

The existing dirty worktree contains a separate C++26 reflection and
machine-description feature, which this migration preserves. The Nix flake now
supplies ELFIO 3.12, GoogleTest 1.17.0, and all lint tools. It exposes their
CMake package paths to both the development shell and apps. `orch.sh` runs the
tools directly and no longer generates or consumes a Conan toolchain. The
Conan and uv manifests, profiles, and locks are removed. CMake keeps using the
upstream ELFIO and GoogleTest targets.

## Validation (2026-09-28)

- `nix flake check --no-build` passed after the final flake change. It reported
  only that the two apps lack optional `meta` attributes.
- `nix develop --command ./orch.sh lint` passed after selecting the original
  `cmake-lint` executable included in Nix's `cmake-format` package. An earlier
  run with the separate `cmake-lint` package failed on existing indentation,
  because that package uses a different linter.
- `nix run .#lint` passed.
- `nix run .#check` configured a fresh `build/nix-gcc-Release` directory,
  built with GCC 16.2, and passed all 35 CTest cases, including the RISC-V
  end-to-end test and reflection smoke test.
- `nix develop --command ./orch.sh config --preset base_with_tests --build-dir
  build/nix-shell-Release` configured a second fresh directory and found the
  Nix-supplied compiler, GoogleTest, ELFIO, and RISC-V cross compiler.
- An independent review of the integrated diff found no actionable issues;
  `git diff --check` passed.

The migration is complete. Existing Conan CMake caches should be replaced with
fresh build directories when using the new workflow.
