# MemBoost Pro v1.0.0 — targeted Linux desktop release

Release date: 2026-11-22. Scope: **Debian GNU/Linux 13 x86-64** and comparable compatible environments, with native X11 or XWayland for graphical mode. This is a stable baseline **for the tested target only**, not a promise of compatibility or performance improvements on all Linux distributions or kernels.

## Included
- Native C++17 X11 desktop interface with five tabs: Overview, Processes, Profiles, Setup guide, Activity.
- Read-only RAM, swap, PSI, zram and top process observations.
- Profile selection with explicit confirmation and PolicyKit elevation only when needed; two temporary kernel parameters, readback and best-effort rollback.
- Read-only `memboost --self-test`, `--json`, `--status`, and `--recommend`.
- Offline public-source audit, deterministic source archive, release checksums, CLI stress and GUI interaction testing.

## Download and install
Choose the `.deb` for Debian 13 amd64 or the `.tar.gz` for compatible x86-64 Linux systems with the required system libraries; build from source for older/distinct distributions. The source and portable archives do **not** remove the host system's library requirements.

```sh
sudo apt install ./MemBoost-Pro-1.0.0-linux-amd64.deb
memboost --self-test
memboost --gui
```

## Risks and limitations
- Never run the GUI with sudo. Only selected, user-confirmed profile writes require administrator privileges.
- Requires libX11 and an X11/XWayland display for the GUI; the CLI can be built without X11.
- No zram activation, swap cycling, cache dropping, OOM killer or persistent daemon.
- RAM pressure and workload-specific tuning are subjective; application does not 'create' RAM or guarantee faster performance.
- Independent multi-distro and disposable-VM root-permission tests have not been completed. Do not claim universal Linux support.
- The checked-in package is built against a modern glibc (not older Ubuntu/Debian releases); inspect DEB dependencies before installation.
- Checksums verify file integrity, **not author identity**. GPG/Cosign signatures are not provided.
- GitHub Releases asset publication requires an authenticated release path; repository-hosted downloads work through README links after the repository is made public.

## Test evidence and feedback
See `docs/RELEASE_VALIDATION_v1.0.0.md` for exact local build/test results and tracked CI statuses after publication. Please report defects in GitHub Issues using distro/kernel, session type and redacted logs. Avoid sharing private process names.
