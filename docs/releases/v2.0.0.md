# MemBoost Pro v2.0.0 — native Linux diagnostics upgrade

This release advances read-only safety and resource-constraint visibility while preserving conservative, temporary tuning.

## New

- Dedicated graphical **Diagnosis** screen with PSI pressure status, cgroup v2 current/limit usage, and diagnostic suggestions.
- `memboost --diagnose` read-only report and bounded `memboost --watch --samples N --interval-ms MS` monitoring.
- cgroup v2 memory and swap counters with namespace-aware detection and unlimited-limit support.
- JSON schema v2 for cgroup and diagnosis information, keeping the v1 counters for basic consumers.
- New parsing/diagnostic/CLI stress tests, real GUI tab interaction and invalid watch argument tests.
- Dedicated **Ubuntu 24.04 Linux VM** CircleCI install/uninstall test, with controlled temporary sysctl profile testing and automatic restoration where VM permissions allow.
- `.github/CODEOWNERS`, contributor governance documentation and an explicit owner-only release approval gate.

## Boundaries and safety

No automatic cache dropping, swap cycling, OOM policy changes, zram configuration, boot-time persistence, or process termination. `--apply PROFILE --yes` remains privileged, explicit and reversible on failure to the extent read-back succeeds. PSI diagnosis is advisory and should not be equated with guaranteed performance improvement.

## Build compatibility

Linux x86-64 `.deb` is built on Debian 13 and requires compatible libc/libstdc++/X11 runtime libraries. `.tar.gz` contains a Linux binary, not a Windows `.exe`. Build from source on distributions with incompatible glibc versions. The project is not certified for all kernels, architectures or Wayland-only compositors.

## Governance

This candidate is prepared on a **review branch**. The owner must review the PR and license decision before merging to `main` and before making repository public. Until then, these are reviewable candidate packages, not a public GitHub Release.
