# Machine descriptions and compile-time records

This document is the design reference and implementation tracker for the
`coregear.tbl` record system and the `coregear.mdesc` instruction model. Both
are C++26 modules. A compiler with P2996 reflection, P3394 annotations,
splicing, C++ modules, and `import std` is required. The selected toolchain is
GCC 16.2 from the Nixpkgs revision in `flake.lock` (`c043004d1c6985732bcc1cbc5a9c9aecbbb4e0f0`), with `-freflection`.
The Nix flake also supplies the C++ libraries and development tools.

## Boundaries and public model

`coregear.tbl` owns compile-time record composition. It knows nothing about
instructions. Its public vocabulary is `tbl::field<T>`, `tbl::bits<N>`,
`tbl::record_class<Bases...>`, `tbl::definition<Bases...>`, `tbl::records`,
`tbl::record_set`, `tbl::database`, `tbl::make_database`, expressions, queries,
and parameterized record-set factories. Public identifiers do not end in `_`.
Only definitions enter databases; record classes supply reusable schema and
defaults. A generated definition without a reflected C++ identifier supplies
an explicit structural name.

`coregear.mdesc` owns machine-independent machine, extension, register,
register-class, operand, format, encoding, instruction, and semantic-action
records. `machine_description<Database>` turns a completed `tbl` database into
immutable instruction and operand descriptors, metadata lookup, a deterministic
decoder, typed operand extraction, and action dispatch. It validates identities,
references, ranges, mappings, action signatures, and encoding overlap during
constant evaluation. Initially decoding searches descriptors in stable database
order; optimized decode trees are deferred.

Illustrative record syntax (field declarations use the one `field<T>` type):

```cpp
struct Base : tbl::record_class<> {
  [[=tbl::required]] tbl::field<int> width;
  tbl::field<int> lanes = 1;
};
struct Wide : tbl::definition<Base> {
  [[=tbl::override]] tbl::field<int> width = 64;
};
```

The supported annotations are `[[=tbl::required]]`, `[[=tbl::final]]`,
`[[=tbl::override]]`, `[[=tbl::append]]`, `[[=tbl::prepend]]`, and
`[[=tbl::override_bits<High, Low>]]`. Append/prepend require compatible list or
string fields. Bit overrides require an in-range inclusive slice of a compatible
`bits<N>` field. Contradictory or unsupported combinations fail compilation.
`field<T>` has a structural unset state, distinct from a value-initialized `T`.

## Field-resolution algorithm

For each concrete definition, the implementation performs these steps in order:

1. Instantiate each parent with its C++ template arguments. Recursively
   collect its ancestry and direct reflected members, preserving declaration
   provenance, name, `field<T>` type, initializer, and annotations.
2. Merge parent fields by name. An unchanged diamond contribution is one field.
   Different changes from independent branches remain an unresolved conflict;
   neither branch silently wins. Same-name declarations with different
   `field<T>` types are errors.
3. Apply scoped `tbl::let` bindings from outer to inner scope to the record set.
   A later inner binding wins. `tbl::set`, `tbl::append_to`, `tbl::prepend_to`,
   and `tbl::set_bits` may resolve inherited conflicts. Scopes cannot alter
   template arguments or modify final fields.
4. Apply direct members. A member with an inherited name must carry
   `tbl::override`, target the identical `field<T>` type, and target a non-final
   field. Its initializer replaces the inherited initializer, preserving field
   identity and inherited metadata; it may add `final` but cannot remove it.
   A valid local override resolves a parent conflict. An override without a
   target and unmarked hiding are errors. A new member cannot carry `override`.
   When an override resolves independent parent declarations, its reflected
   origin is the first parent in base declaration order; inherited `required`
   and `final` constraints are combined across all parents.
5. Resolve typed expressions against the completed record, so references see
   final overridden values. Expressions include field and record references,
   arithmetic, comparisons, conditionals, bit operations and concatenation,
   and list operations. Detect dependency cycles and type mismatches. An unset
   dependency propagates unset. Missing record references are errors.
6. Validate required fields, annotation constraints, and domain constraints.
   A required field still unset at this point is an error.

The database preserves definition order and immutable completed values.
Queries can look up by name, iterate, filter by ancestry or predicate, and
project fields. Duplicate concrete names are errors. Parameterized factories
expand record sets before database materialization and may conditionally emit
definitions.

## Expression contract (MD-04)

Expressions are structural typed values stored in `field<T>` and evaluated by
`resolved_value<Record, "field">()` after inherited fields are collected.
`literal(value)` supplies a constant; `ref<T, "field">` reads the completed
current record. `record_ref<T, Definition, "field">` reads a concrete definition
by C++ type. Named database references and membership validation belong to
MD-07. Reference value types must match exactly. Dependencies are identified by
both record type and field name, including across record references.

The operator set comprises binary `+`, `-`, `*`, `/`, `%`, unary `-`, the named
comparisons `equal`, `not_equal`, `less`, `less_equal`, `greater`, and
`greater_equal`, and `select(condition, yes, no)`. Bit expressions provide
`bit_and`, `bit_or`, `bit_xor`, `bit_not`, `shift_left`, `shift_right`, inclusive
`slice_bits<High, Low>`, and `concat_bits` (left operand supplies high bits).
Fixed-size `std::array` lists provide `concat_list`, `list_at<Index>`, and
`list_size`.

Bit shifts preserve the operand width, discard shifted-out bits, and return
zero for counts at least that width. Negative counts are errors. Slices and
list indices must be in range, and bit concatenation cannot exceed 64 bits.

An unset operand propagates unset. A conditional with an unset condition is
unset; otherwise only its selected branch supplies the value. Both branches
must still be statically well-formed. Missing fields, incompatible reference
types, dependency cycles, and invalid record targets are compilation errors.
Expression evaluation uses the completed local override initializer. Scoped
binding precedence remains MD-06 integration work.

## Machine-description rules

Encodings provide fixed mask/value bits and operand mappings. A mapping may
be contiguous or fragmented, insert zero bits, and sign-extend. Operand
descriptors state direction and access behavior. Two instructions whose fixed
bits can match the same word are ambiguous unless their extension conditions
make them mutually exclusive. Every mapped bit must be in range, every
instruction identity must be unique, and referenced extensions, operands,
formats, and actions must exist. An action's simulator and operand signature
must match its instruction. The generated descriptor is the sole source for
names, extension metadata, printing, decoding, and dispatch after migration.

`coregear.fsim.sim` keeps transitional aliases and re-exports while RISC-V
consumers move to `coregear.mdesc`; they are removed once no consumers remain,
unless an external compatibility period is separately approved. Existing
decoded-instruction printing is preserved throughout migration.

## Implementation tracker

Tasks are completed in order. Each implementation task gets adjacent focused
tests, with invalid cases compiled as negative CTest cases. After source,
CMake, shell, or documentation edits, run `./orch.sh lint` inside `nix develop`.
Behavior changes get the narrowest relevant build and test. MD-11 onward also
runs the RISC-V end-to-end test; MD-13 runs the full pinned Nix workflow.

- [x] **MD-00 — Record the design.** Define boundaries, APIs, precedence,
  validation, compatibility, deferred work, and acceptance criteria here.
- [x] **MD-01 — Pin the C++26 reflection toolchain.** Select GCC 16.2 from the
  locked Nixpkgs revision, add a reflection/annotation/splicing/modules/`std`
  smoke target, set C++26 and `-freflection`, and replace incompatible CI jobs.
  Complete when the smoke target builds and runs through Nix.
- [x] **MD-02 — Field and annotation foundations.** Add `coregear.tbl`,
  structural unset `field<T>` and `bits<N>`, annotation tags, and reflected
  direct-member descriptors. Test valid and invalid annotation combinations.
- [x] **MD-03 — Record classes and inheritance.** Collect ancestry and fields,
  substitute template arguments, deduplicate diamonds, retain conflicts, and
  reject type clashes. Test abstract and concrete cases.
- [x] **MD-04 — Expressions.** Add typed structural nodes, late evaluation,
  dependency cycles, unset propagation, and bit/list operations.
- [x] **MD-05 — Local overrides.** Enforce explicit identical-type override,
  final-field protection, inherited metadata, and conflict resolution.
- [ ] **MD-06 — Scoped `let`.** Implement ordered scopes and set/append/prepend/
  bit-slice transformations, including validation.
- [ ] **MD-07 — Databases and queries.** Materialize immutable definitions,
  lookup/filter/project, references, generated records, names, and ordering.
- [ ] **MD-08 — Machine schemas.** Add `coregear.mdesc` record classes and
  domain validators; express a synthetic ISA as a `tbl` database.
- [ ] **MD-09 — Decode and dispatch.** Generate descriptors, deterministic
  decode, operand extraction, ambiguity checks, and typed actions.
- [ ] **MD-10 — Generic API migration.** Move machine-independent concepts from
  `coregear.fsim.sim`, retaining temporary compatibility exports.
- [ ] **MD-11 — RISC-V vertical slice.** Model R/I/S/B/U/J/system/CSR formats,
  bind representative actions, and compare new versus legacy decode.
- [ ] **MD-12 — Full RISC-V migration.** Convert RV32I, RV32M, and Zicsr and
  execute exclusively through generated descriptors.
- [ ] **MD-13 — Remove legacy machinery.** Delete duplicate maps/macros and
  obsolete aliases, finalize docs, and run lint plus full pinned Nix workflow.

MD-04 may start after MD-02 while MD-03 progresses, but integration waits for
stable inherited descriptors. Each task is independently reviewable. Deferred
features are `.td` parsing, a C++23 fallback, variable-length decoding,
assembly generation, and an embedded semantics language.

MD-04 implementation and independent review are complete. Typed record
references, arithmetic and comparisons, conditional evaluation, bit/list
operations, and record-qualified dependency cycle detection have adjacent
positive and diagnostic-specific negative tests. All required checks passed.

## Session handoff (2026-09-28)

The feature branch builds on the separate Nix migration commit and contains
MD-00 through MD-05 as separate commits. MD-04 adds the expression
contract above, implementation, edge-case tests, and eight negative cases;
the negative-test harness also rejects unknown diagnostic expectations.
Independent review found no outstanding correctness issues after fixing
conditional cycle detection and integral shift-count handling.

Validation of the completed MD-04 behavior:

- `nix develop --command ./orch.sh build --build-dir build/Release` passed.
- `nix develop --command ./orch.sh build --build-dir build/Release --target tbl_expressions_test`
  passed after the final behavior changes.
- `nix develop --command ctest --test-dir build/Release -R '^tbl_' --output-on-failure`
  passed all 18 adjacent tests, including 15 diagnostic-specific negative cases.
- `nix develop --command ./orch.sh lint` passed all stages after focused
  C++/CMake formatting and CMake variable-name/line-length fixes.

MD-05 selects local override initializers in resolved schemas and expressions,
retains inherited provenance and metadata, rejects attempts to replace final
fields, and resolves parent conflicts. Adjacent tests cover transitive
overrides, diamonds, expression reads, metadata inheritance, and specific
invalid declarations. Independent review found no correctness defect and
identified additional inheritance cases, which were added. The initial lint
run found formatting issues in MD-05 files; those were corrected before final
validation. Validation: `nix develop --command ./orch.sh build --build-dir
build/Release` passed; `nix develop --command ctest --test-dir build/Release -R
'^tbl_' --output-on-failure` passed all 25 cases; and `nix develop --command
./orch.sh lint` passed. No dependency locks or unrelated source files changed
for MD-05.

Next step: MD-06 scoped `let`. Database-name references remain MD-07. Workflow
lesson: recursive parent schemas must preserve both the original field identity
and the member supplying the effective initializer.

Final branch verification: `nix run .#check` configured, built, and passed all
35 CTest cases, including the RISC-V end-to-end test, reflection smoke test,
and 25 `tbl` tests. `nix run .#lint` passed. The MD-02, MD-03, and MD-04
intermediate snapshots also passed their focused builds and tests before their
commits. No blocker remains for MD-06.
