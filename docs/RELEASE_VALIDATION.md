# MemBoost Pro v0.1.1 — release validation (2026-11-22)

## Supported tested baseline
- Debian GNU/Linux 13 (trixie) x86-64; GCC 14.2, CMake, libX11/Xvfb.
- Source compiled into native CLI and GUI-capable C++17 executables.
- Read-only mode by default. Unprivileged CLI tests perform **no kernel writes**.

## Checks executed on local Linux environment
- Release GUI build: CTest **8/8 passed**.
- Release headless build: CTest **8/8 passed**.
- AddressSanitizer and UndefinedBehaviorSanitizer debug build: CTest **8/8 passed** with fail-fast settings.
- Virtual X11 session: application ran until deliberately timed out at 5 seconds (exit 124), no immediate crash reported.
- Live /proc JSON parsed and invariants checked (RAM and swap availability).
- DEB extracted and embedded executable checked for version 0.1.1 and valid JSON monitoring.
- TGZ extracted and embedded executable checked for version 0.1.1.
- Headless CMake install checked to have **no unusable GUI .desktop launcher**.
- Invalid profiles, missing --yes and unrecognized command flags tested to reject changes.
- Profile tuning tests use *temporary fixture files* only, not host /proc/sys.

## Release limitations and outstanding gates
- No privileged kernel writes or forced memory-pressure stress tests run on a physical host/VM.
- No separate VM, cross-distribution, older glibc, ARM or Wayland-only test matrix completed.
- CircleCI project enrollment and pipeline execution have not been observed.
- No release cryptographic signature or official GitHub Releases entry created.
- CircleCI first reported a failure. A matching broken-pipe failure was reproduced locally in package verification under `set -o pipefail`; remote job logs are unavailable, so the precise remote failing step is not confirmed. Replaced the local-failing pipeline with a saved listing; the corrected exact local CI gate passed. Successful corrected CircleCI run is still unverified.
- The private repo publishes a locally verified **limited-target installable release** via tracked GitHub download files; not a universal production certification.
