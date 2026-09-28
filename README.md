# CoreGear – The overall simulator platform

CoreGear is a C++23 simulator platform built with CMake. Nix pins the Linux
toolchain, C++ dependencies, and development tools; CMake configures and builds
the project.

## Development

Enter the reproducible development environment with Nix:

```sh
nix develop
```

Inside it, use the orchestration commands:

```sh
./orch.sh lint
./orch.sh everything --preset base_with_tests --lint
```

For non-interactive runs, Nix provides the same workflows directly:

```sh
nix run .#lint
nix run .#check
```

`flake.lock` pins the dependency set. Update it only for an intentional
dependency change. Use a fresh build directory after switching from a Conan
build, because CMake caches toolchain and package paths.
See [AGENTS.md](AGENTS.md) for the full workflow.
