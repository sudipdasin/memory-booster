# Public beta testing guide — MemBoost Pro v1.0.0

The following commands are read-only (except for installation):

```sh
sudo apt install ./MemBoost-Pro-1.0.0-linux-amd64.deb
memboost --version
memboost --self-test
memboost --status
memboost --json
memboost --gui
```

Check that the five tabs render and work; confirm the Profiles tab shows a confirmation dialog and cancel it. Do **not** accept a tuning change merely to test the interface unless you are using a disposable VM with a recovery path. To test a safe preview without any changes:

```sh
memboost --apply balanced --dry-run
```

Uninstall with `sudo apt remove memboost-pro`. If installed through the tarball, remove only the files you extracted or installed manually. File issues with the output of `memboost --self-test`, distribution and kernel details, and screenshots that contain **no private process names or account information**. Do not publicly post the `--json` process list unredacted.

## Smart Optimize and premium GUI (3.2.0)

Run `memboost --optimize --dry-run` and ensure it never changes `vm.swappiness` or `vm.vfs_cache_pressure`. The `core_unit_tests` cover healthy PSI, moderately elevated PSI with adequate headroom, critical PSI, low RAM headroom, disabled swap, cgroup pressure, unsupported PSI, bad sysctl fields and already-responsive settings.

The `gui_interaction` test uses Xvfb to click all seven sidebar views, open the prominent Optimize Memory preview, cancel, select a profile, preview/cancel the profile, and close cleanly without tuning the host. On real hardware, evaluate reported PSI and latency with a stable workload; do not claim a performance gain from memory percentage alone.
