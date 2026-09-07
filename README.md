# CoreGear – The overall simulator platform

CoreGear is a C++23 simulator platform built with CMake and Conan. Nix provides
the pinned Linux development environment; Conan resolves C++
dependencies; and CMake configures and builds the project.

## Development

Enter the reproducible development environment with Nix:

```sh
nix develop
```

Inside it, use the existing orchestration commands:

```sh
./orch.sh lint
./orch.sh everything --preset base_with_tests --lint
```

For non-interactive runs, Nix provides the same workflows directly:

```sh
nix run .#lint
nix run .#check
nix run .#check-clang
```

`flake.lock`, `uv.lock`, and `conan.lock` are reproducibility inputs. Update
each lockfile only when intentionally changing its corresponding dependencies.
See [AGENTS.md](AGENTS.md) for the full workflow.
