# Security policy

## Reporting a vulnerability

Please do not open a public issue for a suspected vulnerability. Report it privately to the maintainers with:

- a concise description of impact and affected revision;
- reproducible steps or a minimal proof of concept; and
- any suggested mitigation.

Maintainers will acknowledge the report, assess the impact, and coordinate a fix before public disclosure. Do not include secrets or private credentials in a report.

## Development expectations

Changes that introduce external input, binary parsing, process execution, filesystem access, or dependency updates should include focused validation and tests. CI workflows run with read-only repository contents permissions unless a job explicitly needs more.
