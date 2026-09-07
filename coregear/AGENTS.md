# Core source guide

Read the repository-level `AGENTS.md` first. This guide adds source-layout
rules for CoreGear's canonical source directory, based on
[P1204](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1204r0.html):
keep related source and unit tests together; do not split them into `include/`
and `src/` trees.

## Layout and naming

- Organize code by component beneath `coregear/` (for example,
  `coregear/fsim/riscv/`), and update the nearest `CMakeLists.txt` when adding
  a target or source file.
- Keep module interfaces (`.mpp`), implementations (`.cpp`), headers (`.hpp`),
  and unit tests (`.test.cpp`) together.
- Name exported modules with the `coregear` prefix and mirror the directory
  hierarchy: for example, `coregear.fsim.riscv` is defined in
  `coregear/fsim/riscv/riscv.mpp`. Use module partitions for component-private
  pieces where appropriate.
- Include project headers with angle brackets and their `coregear/` path, such
  as `<coregear/utility/assert.hpp>`.
- Use names containing only letters, digits, `_`, and `-`; reserve `.` for file
  extensions and C++ module-name separators.

## Tests

- Add unit tests as adjacent `*.test.cpp` files and register them under
  `CG_ENABLE_TESTING` in the component's `CMakeLists.txt`.
- Repository end-to-end tests and their CTest registration are in the root
  `test/` directory (the repository-specific singular-name exception to
  P1204's `tests/` example).
