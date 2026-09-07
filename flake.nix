{
  description = "Reproducible development environment for CoreGear";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      llvm = pkgs.llvmPackages_21;
      riscvGcc = pkgs.pkgsCross.riscv64-embedded.stdenv.cc;
      commonPackages = [
        pkgs.cmake
        pkgs.gcc15
        llvm.clang
        llvm.clang-tools
        llvm.libcxx
        pkgs.ninja
        riscvGcc
        pkgs.uv
      ];
      mkApp = name: command: pkgs.writeShellApplication {
        inherit name;
        runtimeInputs = commonPackages;
        text = ''
          if [[ ! -x ./orch.sh ]]; then
            echo "Run this command from the CoreGear repository root." >&2
            exit 2
          fi
          export CONAN_HOME="$PWD/.cache/conan"
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
        shellHook = ''
          export CONAN_HOME="$PWD/.cache/conan"
        '';
      };

      apps.${system} = {
        lint = {
          type = "app";
          program = "${mkApp "coregear-lint" "./orch.sh uv --sync && ./orch.sh lint"}/bin/coregear-lint";
        };
        check = {
          type = "app";
          program = "${mkApp "coregear-check" "./orch.sh everything --preset base_with_tests --build-dir build/nix-gcc-Release"}/bin/coregear-check";
        };
        check-clang = {
          type = "app";
          # libc++'s experimental `std` module cannot export the fortified
          # glibc stdio overloads with Clang 21. Retain the remaining Nix
          # hardening settings, but omit the incompatible fortify variants.
          program = "${mkApp "coregear-check-clang" "NIX_HARDENING_ENABLE='bindnow format libcxxhardeningfast pic relro stackclashprotection stackprotector strictflexarrays1 strictoverflow zerocallusedregs' CONAN_HOME=\"$PWD/.cache/conan-clang\" CXXFLAGS='-nostdinc++ -isystem${llvm.libcxx.dev}/include/c++/v1' LDFLAGS='-L${llvm.libcxx}/lib -Wl,-rpath,${llvm.libcxx}/lib' CG_CXX_STDLIB_MODULES_JSON=${llvm.libcxx}/lib/libc++.modules.json CG_CXX_STDLIB_INCLUDE_DIR=${llvm.libcxx.dev}/include/c++/v1 CG_CXX_STDLIB_LIBRARY_DIR=${llvm.libcxx}/lib ./orch.sh everything --preset base_clang_with_tests --profile clang.txt --build-dir build/nix-clang-Release"}/bin/coregear-check-clang";
        };
      };
    };
}
