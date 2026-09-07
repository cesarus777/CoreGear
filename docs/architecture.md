# Architecture direction

CoreGear is being designed as an extensible framework for modelling hardware
blocks and their composition: compute units (CUs) such as CPUs, GPUs, TPUs,
and future accelerators; memory systems; interconnect; and other platform
components. This is a target architecture, not a claim that every capability
already exists.

## Current component map

The current source tree establishes these module families. Component paths are
relative to `coregear/`.

### `coregear`

| Component | Module | Responsibility |
| --- | --- | --- |
| `common` | `coregear.common` | Shared basic types, bit utilities, and project exceptions. |
| `utility` | `coregear.utility` | Assertions, logging, unreachable handling, and generic C++ helpers. |

### `coregear.fsim`

| Component | C++ module | Responsibility |
| --- | --- | --- |
| `fsim/sim` | `coregear.fsim.sim` | Reusable instruction, operand, opcode, and decode-description machinery. |
| `fsim/memory` | `coregear.fsim.memory` | In-memory storage and typed load/store operations. |
| `fsim/riscv` | `coregear.fsim.riscv` | The current RISC-V functional model, including instruction extensions, registers, and ELF loading. |

The `cg_riscv_sim` binary is built from `fsim/riscv/main.cpp` and exposes the
RISC-V model as a command-line simulator. `test/` builds a minimal RISC-V image
and validates it through that binary.
`fsim` is the present functional-simulation family. New block models should be
peer components with a clear responsibility, not folded into the RISC-V
implementation. A component's `CMakeLists.txt` is the source of truth for its
build-target dependencies.

## Model composition

The framework must support both functional models, which establish externally
observable behavior, and precise models, which add selected detail such as
timing, microarchitecture, memory hierarchy, or resource use. Models of
different fidelity must be composable in one simulation; adopting a precise
model for one block must not require every other block to be equally precise.

Each model should state the fidelity dimensions it provides rather than use an
ambiguous single "precise" flag. Its public contract must make clear which
state transitions, interactions, ordering, and timing assumptions it models.

## Extensibility rules

- Core abstractions describe compute, state, memory, communication, and
  scheduling without assuming a CPU-specific execution model or instruction
  set.
- Hardware-block-specific logic belongs in a dedicated component behind those
  abstractions. A new accelerator, interconnect, or memory-system model should
  extend the framework rather than require unrelated components to learn its
  internal representation.
- Components interact through explicit, deterministic interfaces. Do not rely
  on hidden global state or incidental ordering between models.
- Preserve the functional contract when adding detail. A more precise model may
  refine how and when behavior occurs, but must not silently change behavior
  outside its declared fidelity contract.
- Keep model configuration explicit and reproducible so the selected CU models
  and fidelity levels can be identified from a simulation run.

## Change guidance

When adding a model or cross-component feature, document the model boundary,
its fidelity dimensions, its dependencies, and its test strategy. Add focused
unit tests for model behavior and integration tests for interactions between
models of different fidelity.
