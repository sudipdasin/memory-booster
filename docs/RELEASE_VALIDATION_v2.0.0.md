# v2.0.0 verification ledger — tested build versus pending VM gate

## Executed locally on Debian GNU/Linux 13 (x86-64)

- GCC Release / native X11 GUI: **15/15 CTest passed** twice (including actual Xvfb GUI tab interaction, modal cancellation, window closing and real CLI safety stress).
- Clang Release / native X11 GUI: **15/15 CTest passed**.
- GCC AddressSanitizer + UndefinedBehaviorSanitizer headless Debug: **14/14 CTest passed** (ASAN detect_leaks=1, halt_on_error=1, UBSAN halt_on_error=1).
- Separate headless rebuild from the distributed versioned source tarball: **14/14 passed**.
- Actual extracted DEB binary: `--version`, `--self-test`, `--diagnose` and virtual X11 GUI interaction passed.
- Versioned ZIP: all expected members present, CRC test passed. DEB/TGZ/source archive extracted safely; no absolute paths, symlinks, or path traversal detected in tar members.
- SHA-256 checksums for `.deb`, `.tar.gz`, source archive verified; offline public source audit passed.
- Tests reject unconfirmed kernel tuning, malformed monitoring arguments, invalid profiles and unsafe memory parsing; all actual CLI integration writes to host sysctl were checked unchanged.

## GitHub / independent VM (must be verified separately)

- CircleCI VM executor: Ubuntu 24.04 installation, actual GUI and package-uninstall job configured; **status pending until review branch pipeline executes**. The VM job must report success before any public release claim.
- GitHub branch protections/codeowner required reviews: policy and CODEOWNERS files prepared, **not yet enforced** until owner configures GitHub ruleset.
- Official GitHub Releases asset publication: **not completed**; requires explicit tag, protected credentials, manual `hold-release` approval and successful jobs.
- Repository visibility: **private** until owner approves license and changes visibility.
- Untested: Wayland without XWayland, ARM, Fedora, Arch/openSUSE installation, older glibc and disk-image signing. These platforms must not be advertised as tested.

## License status

The active package license is still MIT. GPL-3.0-or-later is a proposal only, requiring owner authorization and compatible contributor rights. See `docs/LICENSE_OPTIONS.md`.
