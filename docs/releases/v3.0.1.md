# MemBoost Pro Community v3.0.1 — GPL adoption and public launch hardening

**Release candidate only:** owner review required before merging, public visibility change or official GitHub Release publication.

This release adopts the owner-approved GNU General Public License **version 3 or later** (`GPL-3.0-or-later`) for the Community edition. The distributable source tarball, ZIP bundle and installed Debian package include the full GPLv3 text, a copyright/previous-version license notice and the original inherited MIT terms in `LICENSES/MIT-legacy.txt` (installed as `MIT-legacy.txt`). Every active C++ and Python source file has an SPDX identifier. Prior releases through v3.0.0 keep their MIT grants; they are not silently overwritten or retroactively changed.

Feature set: seven-tab native X11/XWayland interface, memory pressure diagnosis, cgroup-aware resource status, Memory Sentinel paging/major-fault/OOM telemetry, bounded JSONL sampling and explicitly confirmed temporary kernel tuning. No automatic cache dropping, OOM killing or privileged writes.

Source: C++17 + optional X11, CMake >=3.16. Test scope: Debian 13 x86-64 native local; Ubuntu 24.04 independent machine VM builds and installs its own package when CI passes. Do not assume compatibility with all distros, architectures or kernels.

**Install** (for the supported Debian 13 x86-64 build):

```sh
sudo apt install ./MemBoost-Pro-3.0.1-linux-amd64.deb
memboost --self-test
memboost --gui
```

Governance remains separate from software licensing. No PR merge, GitHub visibility change or formal release publication is included in this candidate. See docs/PUBLIC_READINESS.md for the required checks.
