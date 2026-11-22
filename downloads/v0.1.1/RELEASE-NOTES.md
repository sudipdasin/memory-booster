# MemBoost Pro v0.1.1 — limited-target installable release

**Release date:** 2026-11-22  
**Source repository:** https://github.com/sudipdasin/memory-booster  
**Audience:** Authorized users of this private repository, testing on Debian 13 x86-64 or compatible Linux.

## Included
- Native C++17 GUI: Overview, Processes, Profiles, Setup guide and Activity.
- Live RAM, swap, memory pressure (PSI), zram diagnostics and memory-heavy processes.
- CLI status, JSON and dry-run; guarded profiles require confirmation and admin privileges.
- Improved failed-write rollback with readback; no automatic destructive memory operations.
- Eight CTest checks and CircleCI build/test/packaging configuration.
- CircleCI packaging verification updated to avoid a reproducible `pipefail`/`dpkg-deb` broken-pipe error.

## How to install

Download `MemBoost-Pro-0.1.1-linux-amd64.deb` from the [home page](../../README.md#download--install), then:

```sh
sudo apt install ./MemBoost-Pro-0.1.1-linux-amd64.deb
memboost --status
memboost --gui
```

A source TAR.GZ is also available for other supported toolchains. **Check SHA256SUMS.txt before installing.**

## Verification
- Locally: Release GUI CTest 8/8, headless CTest 8/8 and ASan/UBSan CTest 8/8 passed.
- Headless GUI startup via Xvfb, real `/proc` monitoring and unpacked DEB/TGZ executable checks passed.
- Automated CI run, multi-distro VM runs, privileged kernel integration and official GitHub Releases publication **not yet verified**.

## Important limitations
- No universal RAM boost, free memory creation or automatic optimization claim.
- Linux GUI requires X11 or XWayland. Debian 13 build needs newer glibc/libstdc++ than some older distributions.
- Kernel tuning is temporary; users must approve action with PolicyKit or explicit privileged CLI.
- The repository is **private**; license/redistribution has not been granted.
