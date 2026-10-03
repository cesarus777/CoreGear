---
name: github-task-workflow
description: Execute GitHub Issue tasks through GitHub MCP, from assignment and claim through implementation, PR review, and completion.
---

# GitHub task workflow

Use this skill for work assigned from a GitHub Issue. The Issue is the durable
task specification; the harness run is an execution attempt; the PR is the
proposed implementation. One Issue may have multiple runs or PR attempts.

## Ownership and state

- The harness owns deterministic selection, scheduling, concurrency control,
  claiming, retries, and assignment. Do not invent a scheduling policy or pick
  another issue unless explicitly instructed.
- The coding agent reads the assigned issue, inspects code, implements, tests,
  reports useful discoveries, and creates or prepares the PR as appropriate.
- Use existing repository labels or Projects fields for workflow state. If none
  exist, use `ready → working → review → done` as the conceptual lifecycle,
  with `blocked` for exceptions. Do not create labels merely to follow this
  skill. GitHub Issue state and comments hold durable task information; the
  harness holds execution-attempt state.

## Start assigned work

1. Through GitHub MCP, read the issue title, body, acceptance criteria,
   relevant comments, labels or fields, parent and sub-issues when relevant,
   and available blockers or dependencies. Verify that the issue is actionable.
2. Respect the harness's claim or lock. If claiming is delegated to you, use
   an exclusive or conditional GitHub/MCP operation if available, then re-read
   the state before work. If no safe claim is possible, return the issue to the
   harness for coordination rather than risk duplicate assignment.
3. Use a branch associated with the issue, following repository conventions.
   Do not make a local Markdown copy of the issue. Harness-required temporary
   scratch work must not become a committed task database.

## Implement and report

- Treat issue acceptance criteria as the contract. Read applicable repository
  instructions and follow existing implementation patterns; keep unrelated
  changes out of scope.
- Put changes to the durable task specification or concise discoveries and
  blockers on the Issue through GitHub MCP. Do not post chain-of-thought or raw
  agent logs. Keep run details in the harness, not repository task files.
- Run the appropriate repository checks. Create or prepare a PR through the
  normal workflow and link it to the Issue. Use `Fixes #<issue>` when the PR
  fully resolves it. Move to `review` if that workflow state exists. CI and
  review verify the PR before merge. Let the merge close the Issue; do not
  close it early.

## Incomplete attempts

If work cannot finish, do not mark the Issue done. Record an actionable blocker
on the Issue when useful, then use `blocked` or return to `ready` according to
the repository or harness convention. Leave enough context for another attempt
without duplicating internal reasoning. If GitHub MCP is unavailable, report
that to the harness; do not substitute a local task tracker.
