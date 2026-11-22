# Changelog

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
