# Contributing — MemBoost Pro Community

Thank you for helping improve this Linux memory diagnostics project. **GPL-3.0-or-later** applies to new Community contributions accepted into the project. By submitting a PR, contributors represent that they may contribute the code under this license while retaining copyright over their own work. Do not include copyrighted material you cannot sublicense or material incompatible with GPL-3.0-or-later.

All contributions to the canonical repository must be made as pull requests and reviewed by the owner **@sudipdasin** under activated GitHub branch rulesets. Approval is not required to create or develop independent GPL-compliant forks; it is required for changes merged into this official repository. CODEOWNERS is advisory until an administrator enables required code-owner reviews.

Include tests, reproductions and sanitized logs for bugs; avoid private keys, user names, customer information and process names in posted diagnostics. Use CircleCI exclusively, not GitHub Actions. Privileged changes must remain opt-in, reversible and tested on disposable Linux VMs before proposal.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 scripts/public_release_audit.py
```

The repository owner reserves final authority over merges and official release publication, not over legally compliant independent redistribution. Previously distributed MIT versions retain their historic notices. See [copyright policy](COPYRIGHT.md).
