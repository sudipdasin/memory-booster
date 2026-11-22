# MemBoost Pro Community — v3.0.1

**Native C++17 Linux memory diagnostics and conservative, confirmed tuning.** The owner has approved the **GNU General Public License version 3 or later** for this Community release. The GPL-bearing v3.0.1 release has been merged into the official `main` branch. **The owner controls public visibility.** Until the repository is made public, downloads require GitHub access.

## License and public development

**License: [GPL-3.0-or-later](LICENSE)** · [Copyright and original MIT notices](COPYRIGHT.md) · [Preserved historical MIT notice](LICENSES/MIT-legacy.txt) · [Review governance](docs/RELEASE_GOVERNANCE.md)

Anyone may use, modify and redistribute the Community code in compliance with GPL, including commercial redistribution. This does **not** authorize modifying the canonical repository without accepted pull requests. Owner review must be enabled in GitHub branch rules; CODEOWNERS alone is not enforcement.

**Historical release note:** earlier artifacts through v3.0.0 remain available under their original MIT license grants. Changing this checkout's license does not retract previously granted MIT rights or relicense previous binaries retroactively.

## Download MemBoost Pro Community v3.0.1 for Linux

| File | GitHub versioned download |
|---|---|
| Debian 13 x86-64 `.deb` | [Download Debian package](downloads/v3.0.1/MemBoost-Pro-3.0.1-linux-amd64.deb?raw=1) |
| Linux x86-64 `.tar.gz` | [Download portable archive](downloads/v3.0.1/MemBoost-Pro-3.0.1-linux-amd64.tar.gz?raw=1) |
| Complete GPL-covered C++ source | [Download source archive](downloads/v3.0.1/MemBoost-Pro-3.0.1-source.tar.gz?raw=1) |
| DEB, Linux archive, source, GPL and checksum ZIP | [Download ZIP](downloads/v3.0.1/MemBoost-Pro-3.0.1-download-bundle.zip?raw=1) |
| SHA-256 manifest | [Verify downloads](downloads/v3.0.1/SHA256SUMS.txt) |

These exact versioned files are committed to `main` as repository-hosted downloads. They are **not yet attached to a formal GitHub Releases entry**. Once visibility is public, verify anonymous downloads and SHA-256 checksums. The `.deb` targets a tested Debian 13 x86-64 baseline; Ubuntu 24.04 is validated separately using a VM-built package. No universal Linux compatibility or native Windows `.exe` is claimed.

```bash
sudo apt install ./MemBoost-Pro-3.0.1-linux-amd64.deb
memboost --self-test
memboost --gui
```

## Core features

- Seven native X11/XWayland GUI tabs: Overview, Processes, Diagnosis, Telemetry, Profiles, Setup guide, Activity.
- Available RAM, swap, PSI memory pressure and zram, with cgroup v2 limits where available.
- Read-only `/proc/vmstat`, OOM and paging event counters, bounded local telemetry sampling, JSONL output.
- Confirmed, temporary kernel swappiness and VFS pressure profiles with readback verification and rollback on error.
- No automatic cache drops, swap cycling, process killing, unsolicited privileged writes, cloud upload or background services.

![Native seven-tab Linux MemBoost interface](assets/memboost-v3-dashboard.png)

## Read-only CLI and diagnostics

```bash
memboost --status
memboost --json
memboost --diagnose
memboost --signals
memboost --track --samples 5 --interval-ms 1000 --format jsonl > sample.jsonl
memboost --apply balanced --dry-run
```

## Rebuild and verify the public source

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMEMBOOST_BUILD_GUI=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
python3 scripts/public_release_audit.py
python3 scripts/make_release.py --build-dir build-release
```

Requires a C++17 compiler, CMake >=3.16 and optional X11 development headers for native GUI. CLI builds do not require a display server. No guaranteed operation on non-Linux kernels.

## GitHub governance and public launch

The owner-authorized [v2 PR #1](https://github.com/sudipdasin/memory-booster/pull/1), [v3 PR #2](https://github.com/sudipdasin/memory-booster/pull/2), and [GPL v3.0.1 PR #3](https://github.com/sudipdasin/memory-booster/pull/3) have **all merged into main**. Future contributions to the canonical repository must go through owner-approved pull requests. GPL permits forks; it does not give contributors direct write access.

**Owner action before public launch:** Enable an active GitHub ruleset for `main` and `release/*` requiring pull requests, approved reviews, all three CircleCI jobs, and preventing direct pushes, force pushes, branch deletion and unattended merges. Check old Git history, branches and binary archives for sensitive material and confirm third-party licensing. CODEOWNERS by itself does **not** enforce approval. CircleCI remains the exclusive CI provider; GitHub Actions is not used.

See [public launch checklist](docs/PUBLIC_READINESS.md), [release instructions](docs/RELEASE_PUBLISHING.md), [contribution policy](CONTRIBUTING.md) and [security reporting](SECURITY.md).