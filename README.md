# MemBoost Pro

**v1.0.0 — tested Linux memory diagnostics and reversible tuning for Debian 13 x86-64.** This is a targeted stable release, not a universal all-distro certification. Read [known limitations](docs/releases/v1.0.0.md) before installing.

## Download for Linux

| Artifact | Direct repository download |
|---|---|
| **Debian 13 amd64 package** | [Download `.deb`](downloads/v1.0.0/MemBoost-Pro-1.0.0-linux-amd64.deb?raw=1) |
| Linux x86-64 archive | [Download `.tar.gz`](downloads/v1.0.0/MemBoost-Pro-1.0.0-linux-amd64.tar.gz?raw=1) |
| Source code archive | [Download source](downloads/v1.0.0/MemBoost-Pro-1.0.0-source.tar.gz?raw=1) |
| **Complete bundle** | [Download ZIP](downloads/v1.0.0/MemBoost-Pro-1.0.0-download-bundle.zip?raw=1) |
| Verification | [SHA-256 checksums](downloads/v1.0.0/SHA256SUMS.txt) · [Release notes](docs/releases/v1.0.0.md) |

The GitHub repository is **currently private**. These are real, versioned, repository-hosted downloads; after the owner makes the repository public, ordinary visitors can download them without authentication. This is **not** a claim that GitHub Releases assets have been published.

```sh
sudo apt install ./MemBoost-Pro-1.0.0-linux-amd64.deb
memboost --self-test
memboost --gui
```

For the portable archive, unzip/extract it and run the included `usr/bin/memboost` on compatible x86-64 distributions; its system library requirements remain. Build from source on older platforms.

![Real MemBoost Pro dashboard under virtual X11](assets/memboost-overview.png)

## What it does

- **Dashboard:** live RAM availability, actual swap usage, PSI pressure, zram detection and memory history graph.
- **Process viewer:** top resident-memory consumers (read-only, does not kill processes).
- **Profiles:** three explained temporary tuning options for swappiness and VFS cache pressure, with explicit confirmation and PolicyKit privilege elevation.
- **Wizard:** five-step setup guide describing responsible Linux memory optimization.
- **Activity:** on-screen action feedback.
- **CLI:** monitoring and reports through `--status`, `--json`, `--recommend`, `--self-test` and `--apply PROFILE --dry-run`.

Linux uses spare RAM for useful file caches; an application that merely drops caches is not a reliable speed booster. MemBoost does **not** automatically drop caches, cycle swap, start daemons, terminate processes, or modify boot-time settings. Profile changes require administrator approval, are read back and have best-effort rollback on failure. They are not persisted across reboot.

## Compatibility

Verified release target: Debian GNU/Linux 13 x86-64 with X11 or XWayland for GUI. Requires modern glibc (at least 2.38 for the packaged build) and compatible libstdc++. Source builds require CMake 3.16+, C++17, and optionally libX11 development headers. Other distributions/kernels, non-x86-64 platforms and Wayland-only without XWayland remain unverified.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 scripts/public_release_audit.py
```

For a headless build add `-DMEMBOOST_BUILD_GUI=OFF`. For read-only shell testing: `memboost --self-test`. For an **optional** profile preview without changes: `memboost --apply balanced --dry-run`. Never run the GUI as root.

## Engineering and release evidence

Build/test policy: [CircleCI exclusively](docs/CI_PROVIDER_POLICY.md), because GitHub Actions is restricted by billing; no GitHub Actions workflows are enabled. See [validation report](docs/RELEASE_VALIDATION_v1.0.0.md), [public-readiness checklist](docs/PUBLIC_READINESS.md), [official release publishing guide](docs/RELEASE_PUBLISHING.md), [public testing guide](docs/TESTING_GUIDE.md), [security policy](SECURITY.md), [contribution instructions](CONTRIBUTING.md), and [public/private edition separation](docs/EDITION_POLICY.md). Build and release scripts are in `scripts/`.

Do not claim compatibility with all kernels or distribution versions without independent testing. Do not publish unredacted process names in logs or issue reports. Please file reproducible bugs via GitHub Issues.

## License

[MIT License](LICENSE) — permits copying, modification and redistribution, including commercial use. The repository owner should confirm this is the intended license **before changing repository visibility**.
