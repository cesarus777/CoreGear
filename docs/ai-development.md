# Agent development workflow

Use this workflow for substantial features such as the compile-time record and
machine-description system. The main agent owns delivery and may delegate
bounded tasks to subagents. Use one agent for small changes. If delegation is
unavailable, perform the same steps sequentially and report that review was
not independent.

## Ownership and delegation

The main agent selects the next milestone from the feature's existing tracker,
records its acceptance criteria, assigns work, resolves disagreements against
the design, integrates changes, and maintains the session handoff. Continue
through authorized milestones without requiring a new user prompt for each
handoff. Ask about material changes to intent or unresolved design choices.

Use specialists where a separate context adds value:

| Role | Deliverable |
| --- | --- |
| Design reviewer | API and invariant review, ambiguities, and concrete counterexamples. |
| Test reviewer | Cases derived from the contract, missing coverage, and evidence that failures exercise the intended behavior. |
| Implementer | Focused tests and implementation, including debugging and relevant check results. |
| Independent reviewer | Findings against the integrated diff, design, and acceptance criteria, with file references. |

These are task roles, not a requirement to spawn every role for every change.
Start with one implementer and one reviewer; add specialists for specific
uncertainties. Parallelize independent investigations and reviews. Order work
that depends on an unfinished API or another agent's changes.

Give each delegate a compact brief:

```text
Milestone and question:
Design reference and acceptance criteria:
Relevant files and current baseline:
Files allowed to change (or read-only):
Dependencies and check ownership:
Expected artifact and evidence:
```

Delegates return changed files or findings, exact checks and outcomes,
unresolved issues, and the recommended next step. Report a needed scope
expansion to the main agent before editing outside the assignment. The main
agent checks the artifacts and evidence before accepting completion.

## Design, tests, and implementation

1. Resolve the milestone's public contract and invariants in its design
   document. A design reviewer is useful for reflection semantics, inheritance
   precedence, instruction encoding, or hardware-block boundaries.
2. Derive acceptance cases from that contract. For behavior changes, write a
   focused failing test before the implementation where practical. Confirm
   that it fails for the intended missing behavior; a broken toolchain or
   missing import does not establish that. For exploratory compiler support,
   first establish a minimal working probe in the pinned environment.
3. Implement and debug until the relevant tests pass. Keep tests beside their
   component as specified in `coregear/AGENTS.md`. The implementer may refine
   tests, but must explain any change to their expected behavior.
4. Have an independent reviewer inspect the integrated diff and acceptance
   evidence. Resolve actionable findings, rerun affected checks after fixes,
   and record remaining limitations before completing the milestone.

For `coregear.tbl`, test compile-time values and invalid declarations. Negative
compilation tests must distinguish the intended semantic rejection from an
unrelated compiler failure. For the machine-description migration, compare
generated decoding with the existing behavior and run the relevant RISC-V
end-to-end tests. Toolchain changes need a pinned build proving reflection,
annotations, modules, and `import std` work together.

## Shared workspace and verification

Assign one writer per file at a time, including CMake registration and the
tracker. Reviewers inspect a stable milestone snapshot; if it changes during
review, identify the changes that need another look.

Assign one owner for commands that mutate a shared build directory or install
dependencies. Avoid concurrent configure, build, or formatting
commands against the same checkout state. Use the Nix and `orch.sh` commands
in the root guide. The main agent can run verification directly; running a
command does not require a separate agent role.

Record the command, source state it checked, outcome, and any failure that
prevented later stages from running. Local Nix checks and remote CI results
are separate evidence: never report CI passed from a local run. Preserve
pre-existing failures with their location and scope; fix failures introduced
by the assigned change. Only mark a milestone complete when its acceptance
criteria are met and required checks pass, or the user explicitly accepts a
documented exception.

## Durable progress and workflow improvement

Keep one checkpoint in the feature's existing design or task document. Include
the active milestone, accepted decisions, completed work, exact verification
results, blockers, and next action. Update it after integration and before a
session handoff. On resumption, reconcile it with the working tree, including
untracked files. Delegate concise briefs from that state rather than copying
the entire conversation into each task.

The main agent also tracks workflow friction: repeated investigation, missed
requirements, conflicting edits, and unnecessary check runs. Record actionable
lessons in the checkpoint. Change durable guidance when a recurring problem
justifies it; remove obsolete advice as the project changes. After the first
few milestones, assess whether delegation reduced rework and interruptions
enough to justify its time and context cost.
