# Public launch checklist — MemBoost Pro Community 3.0.1

This checklist distinguishes implemented source/CI from actions requiring the owner. The owner-approved GPL-bearing v3.0.1 changes are now merged into `main`; **the owner has now changed repository visibility to public.** The outstanding protection and human provenance/security reviews should be completed promptly; do not mark them done without verification.

## Engineering and licensing

- [x] Owner authorized **GPL-3.0-or-later** for the new Community release.
- [x] Complete GNU GPL v3 license and copyright history included; original pre-v3.0.1 MIT grants are explicitly preserved.
- [x] C++/Python SPDX notices and CMake package licensing metadata updated.
- [x] Build system, read-only CLI, GUI, tests and CircleCI machine VM checks retained; no GitHub Actions.
- [x] Build, sanitizer and Ubuntu 24.04 machine VM CircleCI jobs passed on the GPL v3.0.1 PR head and on the final GPL merge commit (`4dff4788031d763ef3495a88f4f6e0488d1afd2d`). New documentation-only commits require their own final checks.
- [x] DEB/TGZ/source/ZIP uploads matched locally verified bytes; SHA-256 file checks and ZIP integrity passed. Versioned v3.0.1 files are committed to `main`.
- [ ] **Provenance review:** confirm that all included source/artwork/third-party materials may be distributed under the applicable licenses. Automated scans cannot prove ownership.
- [ ] **Post-publication review required:** check entire repository history, branches and binary archives for secrets/PII. Working-tree pattern scanning alone cannot certify historical privacy; rotate any exposed credentials immediately.

## Required owner-controlled GitHub configuration

- [ ] Activate a GitHub ruleset covering `main` and release branches requiring pull requests, required CODEOWNERS approval, no force pushes/deletion, stale review dismissal, and required CircleCI status checks.
- [ ] Configure or explicitly document the needed independent reviewer/solo-maintainer exception; PR authors cannot approve their own PRs.
- [x] Owner-authorized merges completed in dependency order: v2 PR #1, v3 PR #2, GPL v3.0.1 PR #3.
- [x] Owner changed repository visibility to **public**; confirmed using GitHub repository metadata. Public visibility includes the history, branches and downloads.
- [ ] Owner authorizes optional tagged GitHub Releases publication via a protected CircleCI context and manual approval; direct `downloads/v3.0.1/` is a verified fallback but not an official GitHub Release.
- [ ] Independently test public unauthenticated ZIP/DEB/source downloads and SHA-256 checks from a signed-out browser. Versioned files exist on public `main`, but this external HTTP test is still unverified.

## Tested-system scope

Debian 13 x86-64 local native GUI and CLI; Ubuntu 24.04 machine VM builds its own DEB during CI. Fedora, Arch, openSUSE, ARM and Wayland-only environments remain unverified. No Windows `.exe` is provided for this Linux-native tool.

## Privacy reminder

`memboost --json` and GUI process details may contain local application names and PIDs. Do not publish screenshots, logs or archives containing credentials, identifiers, hostnames or sensitive customer data.
