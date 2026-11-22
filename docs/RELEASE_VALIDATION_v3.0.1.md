# MemBoost Pro Community v3.0.1 — validation ledger

This ledger documents executed local checks and remaining public-launch gates. A passing build is not a guarantee of Linux-distro universal compatibility, and a license audit cannot prove third-party copyright ownership.

## Locally executed (Debian 13 x86-64)

- GCC Release GUI build: 20/20 CTest passed, including Xvfb seven-tab navigation and modal-cancel checks.
- Clang Release GUI build: 20/20 CTest passed.
- GCC headless AddressSanitizer/UndefinedBehaviorSanitizer: 19/19 passed.
- Independently rebuilt the distributable GPL-bearing source archive: 19/19 headless tests passed.
- Extracted Debian package and Linux TGZ executable: version and self-test passed, GUI interaction test passed for the Debian executable.
- Extracted `LICENSE` from Debian, TGZ and source archive: byte-identical to the root GPL v3 full text; the original inherited MIT permission notice is preserved separately and the ZIP bundles both texts and copyright history.
- SHA-256 manifest, ZIP CRC and archive traversal/symlink safety checks passed.
- No privileged kernel changes were made during the local license migration tests.

## Independent CircleCI gates for the exact final commit

- [ ] `ci/circleci: build-test-package`
- [ ] `ci/circleci: headless-sanitizers`
- [ ] `ci/circleci: ubuntu-2404-vm-install`

Recheck live statuses on the precise v3.0.1 PR head. The Ubuntu machine VM builds and installs its own package, rather than running the Debian 13 binary. Its privileged kernel tuning substep is conditional on VM permissions and must not be assumed to have run merely because the job passed.

## Before repository becomes public

- [ ] Owner reviews and approves all pending stacked PRs and source-license change; merges in order v2 -> v3 -> v3.0.1.
- [ ] Owner enforces GitHub rulesets to prevent unreviewed merges/direct pushes (CODEOWNERS file alone does not do this).
- [ ] Inspect every history branch, previously distributed archives and dependencies for license compatibility or sensitive data.
- [ ] Owner verifies `main` includes GPL-bearing v3.0.1 binary/source ZIP files and updated homepage before making repo public.
- [ ] Verify unauthenticated public downloads. Formal GitHub Releases assets require separate approved tag and CircleCI credential + hold gate.
