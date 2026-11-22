# Public launch checklist — MemBoost Pro Community 3.0.1

This checklist distinguishes implemented source/CI from actions requiring the owner. The owner-approved GPL-bearing v3.0.1 changes are now merged into `main`; **do not change visibility until owner-managed rulesets and the remaining human privacy/provenance checks are complete.**

## Engineering and licensing

- [x] Owner authorized **GPL-3.0-or-later** for the new Community release.
- [x] Complete GNU GPL v3 license and copyright history included; original pre-v3.0.1 MIT grants are explicitly preserved.
- [x] C++/Python SPDX notices and CMake package licensing metadata updated.
- [x] Build system, read-only CLI, GUI, tests and CircleCI machine VM checks retained; no GitHub Actions.
- [x] Build, sanitizer and Ubuntu 24.04 machine VM CircleCI jobs passed on the GPL v3.0.1 PR head and on the final GPL merge commit (`4dff4788031d763ef3495a88f4f6e0488d1afd2d`). New documentation-only commits require their own final checks.
- [x] DEB/TGZ/source/ZIP uploads matched locally verified bytes; SHA-256 file checks and ZIP integrity passed. Versioned v3.0.1 files are committed to `main`.
- [ ] **Provenance review:** confirm that all included source/artwork/third-party materials may be distributed under the applicable licenses. Automated scans cannot prove ownership.
- [ ] Before public disclosure, check **entire repository history, branches and binary archives** for secrets/PII; working-tree pattern scanning alone cannot certify historical privacy.

## Required owner-controlled GitHub configuration

- [ ] Activate a GitHub ruleset covering `main` and release branches requiring pull requests, required CODEOWNERS approval, no force pushes/deletion, stale review dismissal, and required CircleCI status checks.
- [ ] Configure or explicitly document the needed independent reviewer/solo-maintainer exception; PR authors cannot approve their own PRs.
- [x] Owner-authorized merges completed in dependency order: v2 PR #1, v3 PR #2, GPL v3.0.1 PR #3.
- [ ] Owner explicitly decides to change visibility **after** exact-release verification on `main`. Public becomes accessible to everyone, including historic branches and downloads.
- [ ] Owner authorizes optional tagged GitHub Releases publication via a protected CircleCI context and manual approval; direct `downloads/v3.0.1/` is a verified fallback but not an official GitHub Release.
- [ ] Once public, test unauthenticated GitHub ZIP/DEB/source downloads and SHA-256 checks.

## Tested-system scope

Debian 13 x86-64 local native GUI and CLI; Ubuntu 24.04 machine VM builds its own DEB during CI. Fedora, Arch, openSUSE, ARM and Wayland-only environments remain unverified. No Windows `.exe` is provided for this Linux-native tool.

## Privacy reminder

`memboost --json` and GUI process details may contain local application names and PIDs. Do not publish screenshots, logs or archives containing credentials, identifiers, hostnames or sensitive customer data.
