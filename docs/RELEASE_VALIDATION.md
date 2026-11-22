# MemBoost Pro v0.1.0 — release validation

## Environment

Local build performed on Debian 13 (trixie), Linux x86-64, GCC 14.2.0, CMake, X11 development library, and Xvfb.

## Executed checks

- Release configuration: compiled `memboost_core`, GUI-capable `memboost` and `test_core`
- CTest: 4/4 green (`core_unit_tests`, `cli_status`, `cli_json`, `cli_dry_run`)
- Direct `memboost --status`: parsed real `/proc` and enumerated running processes
- Native GUI started under `xvfb-run` and remained alive through a timed smoke run
- CPack generated `.deb` and `.tar.gz`; `dpkg-deb -I` read package metadata and runtime dependencies

## Not executed or verified

- No privileged kernel write was applied to the host; profile writes were tested only against temporary fixture paths.
- Cross-distro and old-kernel compatibility tests have not yet been completed.
- CircleCI pipeline success, downstream packaging installs, release signing and GitHub Releases publication have not been verified.
- No measured performance improvement or long-term stress stability claim is made.

## Current shipping decision

The local build constitutes an **initial installable v0.1.0 baseline**, not a broadly certified stable production release. Publish a GitHub stable release only after CI, integration and packaging verification on supported targets.
