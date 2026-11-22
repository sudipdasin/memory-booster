# MemBoost Pro — v3.0.0 review candidate

**Native C++17 Linux memory diagnostics, cgroup-aware pressure analysis and cautious temporary tuning.** The official repository is governed by the owner; community changes must go through reviewed pull requests. This branch is a **candidate**, not a universally certified or public GitHub Release.

## Download v3.0.0 candidate

| Artifact | GitHub versioned file |
|---|---|
| Debian 13 x86-64 installer | [Download `.deb`](downloads/v3.0.0/MemBoost-Pro-3.0.0-linux-amd64.deb?raw=1) |
| Linux x86-64 archive | [Download `.tar.gz`](downloads/v3.0.0/MemBoost-Pro-3.0.0-linux-amd64.tar.gz?raw=1) |
| Full source code | [Download source](downloads/v3.0.0/MemBoost-Pro-3.0.0-source.tar.gz?raw=1) |
| Complete package ZIP | [Download ZIP](downloads/v3.0.0/MemBoost-Pro-3.0.0-download-bundle.zip?raw=1) |
| Verification and notes | [SHA-256](downloads/v3.0.0/SHA256SUMS.txt) · [release notes](docs/releases/v3.0.0.md) |

The GitHub repository is **private until the owner makes it public**, and links on a review branch require authorized GitHub access. Direct public downloads can only be verified after publication.

```sh
sudo apt install ./MemBoost-Pro-3.0.0-linux-amd64.deb
memboost --self-test
memboost --gui
```

## Commands

```sh
memboost --status                       # real memory + cgroup stats
memboost --json                         # machine-readable v3 schema
memboost --diagnose                     # PSI + cgroup constraint analysis
memboost --signals                      # cumulative paging, swap and OOM signals
memboost --track --samples 5 --interval-ms 1000 --format jsonl > samples.jsonl
memboost --watch --samples 5 --interval-ms 2000
memboost --apply balanced --dry-run     # harmless preview
# Only after reviewing implications, sudo/pkexec and --yes permit temporary tuning.
```

## Graphical application

Seven native X11 tabs: Overview (RAM/swap history), Processes (read-only RSS), Diagnosis (PSI/cgroup), Telemetry (faults, swap, OOM-event counters and measured rates), Profiles (explicitly confirmed), Setup guide, and Activity. GUI runs as a normal user. An authorization dialog is used only for user-initiated kernel tuning; headless operations work on systems without X11.

![Actual MemBoost native Linux dashboard](assets/memboost-v3-dashboard.png)

## Memory Sentinel telemetry (new in 3.0)

`--signals` reads cumulative Linux VM counters and cgroup v2 `memory.events`. **Cumulative values are not live rates** and can include activity before MemBoost started. `--track` measures intervals and emits plain text or one JSON record per sample (JSONL) for analysis. Counter resets, missing features, invalid parameters, and unavailable streams are handled conservatively; MemBoost does not claim that a rising major-fault rate means an application is faulty. JSON schema moved from 2 to **3** and now includes `vmstat` and `cgroup_events`.

No background service, remote telemetry, process killing, data upload or automatic tuning. JSONL output is written only to stdout unless you choose to redirect it. Its process table can contain program names and PIDs; inspect exports before sharing.

## Safety, scope and compatibility

Linux intentionally uses unused RAM for helpful caches. More available RAM after cache clearing does not necessarily mean better performance, and this program doesn't clear caches automatically. No swap shutdown, zram manipulation, OOM daemon changes or systemd startup changes occur automatically. Profile operations only temporarily set kernel swappiness and VFS cache pressure, verify results and attempt rollback on failure.

The `.deb` is a Debian 13 x86-64 release baseline, **not a universal Linux binary**. Ubuntu 24.04 is tested separately in CircleCI's VM job using its own compiled package. Source requires C++17/CMake 3.16+ (libX11 headers only for GUI). There is no Windows `.exe` because this is a Linux program; use the ZIP/TAR.GZ for portable distribution.

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMEMBOOST_BUILD_GUI=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 scripts/public_release_audit.py
python3 scripts/make_release.py --build-dir build-release
```

CircleCI is the **sole CI provider** under [the existing policy](docs/CI_PROVIDER_POLICY.md). Owner governance: [.github/CODEOWNERS](.github/CODEOWNERS), [review rules](docs/RELEASE_GOVERNANCE.md), [contributing](CONTRIBUTING.md), [security](SECURITY.md), [test guidance](docs/TESTING_GUIDE.md).

## Upgrade sequence and owner approval

This v3.0.0 branch **builds on the unmerged v2.0.0 owner-review branch**. It is submitted as a separate stacked pull request against v2.0.0; merge v2 first and retarget v3 to `main` only after owner approval. Neither branch is automatically merged. CircleCI is the only hosted CI provider; no GitHub Actions are added.

## License — owner's decision pending

Current repository license: [MIT](LICENSE). For a public **copyleft** Community edition we recommend [GPL-3.0-or-later](docs/LICENSE_OPTIONS.md), but that change will not occur without the owner's explicit review. A license cannot require outside forks to obtain approval; branch protection and repo permissions control merges into the canonical project.
