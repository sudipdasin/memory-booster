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
