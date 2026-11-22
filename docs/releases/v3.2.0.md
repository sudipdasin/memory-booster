# MemBoost Pro Community v3.2.0 — Premium UI and Smart Optimize

**Review candidate — owner approval required for merge and formal GitHub release.** Licensed `GPL-3.0-or-later`; historical MIT grants are preserved.

## Highlights

- Rebuilt the Linux GUI into a polished 1280 × 800 native dashboard with sidebar, legible anti-aliased typography via optional runtime Xft, cards, memory graph, badges and prominent **OPTIMIZE MEMORY** action.
- Added a full optimization preview showing exactly why no change is advised, or what optional reversible kernel settings are proposed. The preview never makes changes.
- Added a conservative policy engine that declines tuning during critical pressure, low available RAM, risky cgroup constraints, missing PSI/kernel settings or healthy conditions. The `responsive` profile is only offered under measured moderate pressure, adequate headroom and swap availability.
- New CLI `--optimize [--dry-run]` and `--optimize --yes`, with administrator authorization and a fresh policy evaluation before any potential write. No changes without `--yes` and root access.
- Refined X11 window sizing, rendering cleanup and automated GUI navigation/preview/cancel/close coverage.
- Updated GPL source, documentation, public-readiness guidance and release assets. No GUI font files are redistributed.

## Safety and scope

No automatic process termination, cache dropping, swap cycling, service manipulation, background agent or uploading performance data. An optimization cannot guarantee improved performance across all workloads. Tuning is temporary, has readback verification, and attempts rollback on failure. Unavailable kernel PSI leads to recommendations rather than blind writes.

Supported validation scope: native Debian 13 x86-64 builds and UI integration under Xvfb; independent Ubuntu 24.04 Linux machine-VM compilation, testing and package install/removal in CircleCI when its exact commit succeeds. Other Linux distributions and CPU architectures require separate checks. No Windows EXE; this is a native Linux app.

## Install

```bash
sudo apt install ./MemBoost-Pro-3.2.0-linux-amd64.deb
memboost --self-test
memboost --gui
```

**Formal GitHub Release:** publish only from a verified tagged commit after explicit owner authorization. Repository-hosted bundles are available separately in `downloads/v3.2.0/`.
