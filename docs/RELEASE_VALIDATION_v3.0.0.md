# MemBoost Pro v3.0.0 — verification ledger

This file records test scope, not an assurance that every Linux environment is covered.

## Executed local verification (2026-11-22, Debian 13 x86-64)

- GCC Debug X11: **19/19 CTest passed**.
- GCC Release X11: **19/19 CTest passed**, independent distribution build also **19/19 passed**.
- Clang Release X11: **19/19 passed**.
- GCC headless ASan/UBSan: **18/18 passed**.
- Fresh headless rebuild from the packaged source archive: **18/18 passed**.
- DEB executable: `--version`, `--self-test`, `--signals`, JSONL tracking parsed by Python; X11 interaction test passed.
- TGZ extracted binary: self-test passed; host sysctls remained unchanged.
- ZIP CRC, archive member traversal checks, SHA-256 manifest and public-source scan: passed.

**CI status for the v3 commit is not presumed by this source document.** Check branch CI before merging. No direct VM kernel tuning is claimed from the local container.

## Required local acceptance

- GCC Release, GUI 19/19 CTest; repeat execution and Xvfb navigation.
- Clang Release, GUI 19/19 CTest.
- GCC AddressSanitizer/UndefinedBehaviorSanitizer, headless 18/18 CTest.
- Rebuild from versioned source tarball and run 18/18 CLI tests.
- DEB installation/extraction validation, native executable read-only commands, GUI functional check, TGZ path audit, ZIP CRC and manifest SHA-256.
- `--track` JSONL records parsed by independent Python and no detected changes to `/proc/sys/vm/swappiness` or `/proc/sys/vm/vfs_cache_pressure`.

## Required hosted CI

- CircleCI `build-test-package`: build, 19 tests, source/secret audit, package contents.
- CircleCI `headless-sanitizers`: 18 tests with ASan/UBSan.
- CircleCI `ubuntu-2404-vm-install`: independent Ubuntu 24.04 machine VM, rebuild, install, CLI JSONL, X11 interaction, uninstall.

Check the **exact head commit CI statuses** before approval; a local pass cannot replace the VM checks.

## Owner-only gates

- Existing v2 pull request must be independently reviewed and approved; v3 is stacked onto that branch.
- Enable GitHub required PR/code-owner reviews and required CircleCI status checks. CODEOWNERS alone is not enforced.
- Select and approve the public license, release visibility and publication. This assistant does not change visibility or merge without owner approval.
- Official GitHub Releases uploads need supported permissions/credentials and release-tag workflow approval.
