# Proposed contribution (owner review required)

## Summary

Explain the intended change, user impact and why it is safe on a memory-constrained Linux system.

## Verification

- [ ] Native Linux build + CTest suite passed (include actual results)
- [ ] CircleCI `build-test-package`, `headless-sanitizers` and `ubuntu-2404-vm-install` passed on this exact commit
- [ ] Changes involving privileged behavior were tested on a disposable VM, with rollback evidence
- [ ] No credentials, private process names, personal paths, private keys or sensitive exports committed
- [ ] New source is GPL-3.0-or-later compatible and SPDX-noticed, with contributor rights confirmed
- [ ] User-facing notes, changelog and tests are updated

## Ownership / release gate

CODEOWNERS assigns the owner `@sudipdasin`. An active GitHub branch ruleset must require owner-approved PRs before merge. Submitting or passing CI never authorizes a merge, tag, publication, visibility change or proprietary redistribution. PR authors cannot approve their own changes.
