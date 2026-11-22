# Release validation — v1.0.0

This document records only actually executed checks. The repo CI status should be checked on the exact final release commit before promoting any release claim.

## Local Debian GNU/Linux 13 x86-64 validation
- GCC C++17 GUI build with CTest 12/12 (including actual virtual X11 navigation and read-only CLI stress).
- GCC headless AddressSanitizer/UndefinedBehaviorSanitizer: 11/11 CTest passed. Clang 17 GUI build: 12/12 CTest passed.
- DEB extraction: version, self-test, JSON and full GUI interaction checks passed; TGZ extraction: version/self-test passed; fresh source archive extracted, rebuilt headless and passed 11/11 tests.
- CPack generated DEB and TGZ; SHA-256 manifests verified; public-source audit passed; tar archives checked against absolute/traversal/symlink members; ZIP CRC and six-file contents checked.

## Not yet independently certified
- Installs and kernel-authorized sysctl changes on separate VM images of Ubuntu, Fedora, Arch or openSUSE.
- Wayland-only desktops without XWayland, 32-bit platforms and ARM64.
- GitHub Releases attachment and unauthenticated public downloads before visibility changes.
- Cryptographic signing of binaries and reproducible binary byte-for-byte rebuilds.

## CI restrictions
Only CircleCI is allowed. Releasing to GitHub Releases from a tag requires a protected `GITHUB_TOKEN`, release publishing job and a successful build/test gate; unverified publisher activity must not be reported as completed.

## Package dependency inspection
The `.deb` declares `libc6 >= 2.38`, `libstdc++6 >= 13.1`, and `libx11-6` (plus `libgcc-s1`). It is therefore **not** advertised as universally compatible with older Linux distributions.

## Audit limitations
A regex-based credential scan does not prove absence of all possible sensitive content, but the reviewed source and dashboard artwork contain no visible personal identifiers. Sanity checks against the real Linux host were read-only; privileged kernel modifications were tested exclusively with file fixtures.
