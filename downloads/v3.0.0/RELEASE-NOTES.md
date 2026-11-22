# MemBoost Pro 3.0.0 — Memory Sentinel

**Status: owner-review release candidate (not a public or formal GitHub Release).**

## What's new

- Seventh native Linux X11 **Telemetry** tab reports paging, swap and out-of-memory signals plus measured rates when available.
- New `memboost --signals` for a read-only cumulative memory counter report.
- New `memboost --track --samples 5 --interval-ms 1000 --format jsonl` for bounded interval telemetry exported through stdout, usable for performance comparisons.
- Robust `/proc/vmstat` and cgroup v2 `memory.events` parsers, reset detection, and explicit unavailable-counter reporting.
- JSON snapshot schema **3**; downstream consumers must account for the version change.
- Expanded 19-case CTest suite including GUI automation and CLI safety checks, with compiler/sanitizer CI and an Ubuntu 24.04 VM install job.

## Install and verify

For the tested Debian 13 x86-64 build:

```sh
sudo apt install ./MemBoost-Pro-3.0.0-linux-amd64.deb
memboost --self-test
memboost --signals
memboost --track --samples 3 --interval-ms 500 --format jsonl > samples.jsonl
memboost --gui
```

The source tarball can be rebuilt with CMake 3.16+ and C++17; X11 development headers are optional for the CLI. Installing the Ubuntu package is only claimed after a real Ubuntu VM job passes for the same source commit. **No Windows .exe** is provided; Linux-native kernel interfaces cannot run as an equivalent Windows application.

## Safety and governance

- Read-only by default: no process termination, cache dropping, swapoff, zram reset, boot persistence or network communication.
- Tuning still requires an explicit profile and `--yes` plus privileged authorization, with read-back verification and rollback-on-error.
- Rates derive from two valid samples and a measured monotonic time interval. Missing or reset counters suppress derived rates rather than invent values.
- Keep the repository private and the release in a review branch pending the owner's decision on license, branch rulesets, merge, public visibility and formal release publication.
- GPL-3.0-or-later has been recommended for a copyleft Community edition; **MIT remains the actual license** until the owner approves any change. Repository merge control is enforced by GitHub rulesets, not an open-source license.

## Verification limitations

Local checks and uploaded package checksums are evidence for specific compiler/platform configurations only. Other distributions, Wayland-only sessions, ARM architectures and all Linux kernels are not guaranteed. Real privileged tuning is not assumed to have been tested unless the VM log confirms the writable-kernel substep executed.
