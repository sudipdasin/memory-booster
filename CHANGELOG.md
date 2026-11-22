# Changelog

## 3.0.0 — Memory Sentinel telemetry (owner-review candidate)

- Added native read-only `/proc/vmstat` parser for major page faults, swap-in/out pages, and optional host OOM kill counters.
- Added cgroup v2 `memory.events` counters and safe before/after delta calculation with reset detection and missing-feature handling.
- Added `memboost --signals` and bounded `memboost --track` with structured JSONL output; no network transmission or filesystem writes by default.
- Expanded native Linux X11 GUI from six to seven tabs; Telemetry tab distinguishes cumulative values from per-interval rates.
- Bumped machine-readable snapshot JSON schema to 3; maintained conservative no-automatic-tuning policy.
- Added malformed/overflow parser fixtures, counter reset and timing tests, 7-tab GUI functional test, JSONL integration and VM install gate.
- Preserved existing owner-review branch and manual CircleCI release approval controls; current MIT license unchanged pending explicit owner selection.

# Changelog

## v2.0.0 — release candidate (2026-11-22)

- Native Diagnosis GUI tab, safe PSI/cgroup v2 advice, v2 JSON and `--diagnose` / bounded `--watch` CLI.
- Cgroup v2 namespace-aware sampling, quota and compressed swap visibility.
- 15-test native X11 suite plus extended parser/cgroup/CLI error regression coverage.
- Ubuntu 24.04 independent Linux VM install/uninstall testing in CircleCI; explicit release approval gate.
- CODEOWNERS, review and license decision documents; no automatic relicensing or publication.

# Changelog

## 1.0.0 — 2026-11-22
- Supported-target stable baseline for Debian 13 (x86-64) native GUI and headless CLI. Additional distribution compatibility is not certified.
- Add read-only installation self-test, 32-way concurrent JSON validation, GUI automation, and source-archive packaging controls.
- Establish end-to-end public-release hygiene with MIT license and privacy/credential-source checks.
- Deliver SHA-256-checksummed versioned DEB, Linux tarball, source tarball and ZIP bundle on the repository homepage (no cryptographic signing claim).
- Require separate CircleCI native packaging and headless sanitizer gates; keep GitHub Actions disabled.
- Restrict automatic tuning to explicit confirmed, temporary and verified sysctl profile writes.


## 0.3.0 — 2026-11-22
- Add read-only `--self-test` installation diagnostic.
- Add permissive MIT licensing for potential public source distribution.
- Add native X11 automated end-to-end tab navigation and confirmation-cancel test.
- Add offline public-release audit to catch version mismatches and credential-like material.
- Expand CircleCI verification to headless sanitizer job, GUI interaction test and package audit.
- Keep v0.1.1 packages available until the next release is published.

## 0.2.0 — 2026-11-22
- Reject malformed PSI averages and impossible kernel memory counters rather than asserting false precision.
- Validate old sysctl values before applying a profile and detect close-time write errors.
- Reject extra CLI options, add malformed-input tests, and use randomized private directories for test fixtures.
- Expand CTest safety coverage without privileged host writes.

## 0.1.1 — 2026-11-22
- Add README homepage links to versioned, locally verified release packages.
- Protect overflow-prone MemAvailable fallback and verify full rollback of both kernel values after a failed write.
- Correct version reporting and omit desktop launcher from CLI-only installations.
- Expand CTest to eight checks, including refusal of unsafe/invalid CLI invocations.
- Validate extracted DEB and TGZ binaries; preserve CircleCI-only CI policy.
- Correct a CircleCI package-inspection false failure caused by a broken pipe under `pipefail`.
- Add guarded release publication script for a future authorized CircleCI tag pipeline (not run here).


## 0.1.0 — 2026-11-22

- Establish a native C++17 MemBoost Pro repository and release baseline.
- Replace the incomplete GTK3 draft with a buildable, testable X11 desktop dashboard and headless CLI.
- Add RAM, swap, PSI, zram and top-process observations with accurate `/proc` parsing.
- Add interactive profile selection, privilege-on-demand polkit flow and in-app safety confirmation.
- Add readback-verified, rollback-aware temporary sysctl tuning. No global cache drops, swap cycling or automatic kernel changes.
- Add CTest unit/integration checks, Debian packaging, tarball builds and CircleCI build, test, GUI smoke and packaging configuration.

### Limits

- GUI requires X11 or an XWayland-compatible desktop. Headless builds are supported.
- zram, zswap, oomd and service management are diagnostic-only in v0.1.0.
- 0.1.0 is the initial tested release candidate, not a claim of performance gains on every Linux system.
