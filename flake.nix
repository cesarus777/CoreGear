{
  description = "Reproducible development environment for CoreGear";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      riscvGcc = pkgs.pkgsCross.riscv64-embedded.stdenv.cc;
      dependencies = [
        pkgs.elfio
        pkgs.gtest.dev
      ];
      dependencyPrefixPath = pkgs.lib.concatStringsSep ":" (map toString dependencies);
      commonPackages = [
        pkgs.cmake
        pkgs.cmake-format
        pkgs.gcc16
        pkgs.llvmPackages_21.clang-tools
        pkgs.ninja
        riscvGcc
        pkgs.shellcheck
        pkgs.shfmt
        pkgs.typos
      ] ++ dependencies;
      mkApp = name: command: pkgs.writeShellApplication {
        inherit name;
        runtimeInputs = commonPackages;
        text = ''
          if [[ ! -x ./orch.sh ]]; then
            echo "Run this command from the CoreGear repository root." >&2
            exit 2
          fi
          export CMAKE_PREFIX_PATH="${dependencyPrefixPath}''${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
          export CC=gcc
          export CXX=g++
          ${command}
        '';
      };
    in
    assert pkgs.lib.versionAtLeast pkgs.cmake.version "4.2.3";
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = commonPackages;
        CC = "gcc";
        CXX = "g++";
        CMAKE_PREFIX_PATH = dependencyPrefixPath;
      };

      apps.${system} = {
        lint = {
          type = "app";
          program = "${mkApp "coregear-lint" "./orch.sh lint"}/bin/coregear-lint";
        };
        check = {
          type = "app";
          program = "${mkApp "coregear-check" "./orch.sh everything --preset base_with_tests --build-dir build/nix-gcc-Release"}/bin/coregear-check";
        };
      };
    };
}
