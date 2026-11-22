# MemBoost Pro

Native **C++17** Linux memory diagnostics and guarded tuning with a graphical desktop dashboard and headless CLI. Built for Linux distributions exposing procfs and sysfs, with an X11 GUI (also runs through XWayland where available).

## What works

- Live RAM, swap, memory pressure (PSI), and zram diagnostics
- Largest resident-memory processes, bar graphs and history chart
- A five-tab desktop GUI with Overview, Processes, Profiles, Setup guide and Activity
- Read-only operation by default, no root login required
- Three documented, temporary kernel tuning profiles (only swappiness and VFS cache pressure)
- Confirmation dialog plus PolicyKit elevation at the point of applying a profile
- Kernel value readback and rollback attempts on failure; no system service restarts
- Unit tests, CLI smoke tests, reproducible CMake builds, CircleCI gates and Debian packages

**Caution:** Linux uses spare memory for useful caches; a low 'free RAM' count does not necessarily mean a problem. The app does not claim to create physical RAM or promise a universal speed boost. `zram`, `zswap`, `oomd`, clearing caches, disabling swap and changing overcommit are deliberately not automatic.

## Build

```sh
# Debian / Ubuntu: sudo apt install build-essential cmake libx11-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/memboost --gui
```

For headless Linux: `cmake -S . -B build -DMEMBOOST_BUILD_GUI=OFF`.

## CLI

```sh
memboost --status             # Human-readable read-only snapshot
memboost --json               # Machine-readable snapshot
memboost --recommend          # Snapshot and suggested profiles
memboost --apply balanced --dry-run
pkexec memboost --apply responsive --yes  # Explicit temporary changes
```

Do not launch the GUI with sudo. It runs as a normal user, requesting administrator permission only for controlled kernel writes. Profiles change current runtime settings but **do not persist after reboot**. Before tuning, record your system's baseline.

## Packages

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build && cpack -G DEB && cpack -G TGZ
```

Install generated `.deb` with `sudo apt install ./memboost-pro-*.deb`. For other distros use a CMake source build or the tarball and install the binary as appropriate. A full GUI requires a working X11/XWayland display; Wayland-only systems without XWayland can use the CLI.

## CI

Per [CI provider policy](docs/CI_PROVIDER_POLICY.md), this repository uses **CircleCI exclusively** for automated checks. GitHub Actions is not used. Pushes and pull requests trigger the CircleCI build, test, virtual-display smoke test and packaging jobs if the CircleCI project is enrolled. Build artifacts are uploaded to CircleCI. Release publishing to GitHub requires separate verified release credentials and is not assumed to be configured.

## Security and non-goals

- No shell interpolation or user-provided command execution
- No startup modifications, persistent systemd overrides or hidden daemon
- No swapoff, zram teardown, forced cache flushes or process termination
- Application state is in-memory only; runtime kernel changes reset on reboot
- Rollback is best-effort; if kernel writes or verification fail, inspect with `sysctl vm.swappiness vm.vfs_cache_pressure`
- PSI and zram require kernel support and are detected, not presumed

Report defects using Issues. Include distribution, kernel version, X11/Wayland environment, `memboost --status` output with process names redacted where appropriate, and reproducible steps.

## Source provenance

Rebuilt from the user-supplied `Memboost Pro.txt` initial GTK3 concept to fix compilation and safety defects. This project does not redistribute that draft verbatim.

## Licensing

This repository is private and no redistribution license has been declared. Licensing and any public release require the repository owner's decision.
