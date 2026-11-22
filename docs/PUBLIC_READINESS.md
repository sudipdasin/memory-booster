# MemBoost Pro — public-release readiness

This checklist separates measurable engineering checks from decisions that require the repository owner.

- [x] Project source, tests, and version stored together in GitHub.
- [x] MIT LICENSE included. This permits source redistribution and commercial use; it cannot be unilaterally revoked from recipients of already licensed versions. Review this licensing choice before making the repository public.
- [x] No credentials required for normal application functionality.
- [x] Build is nonprivileged. App monitors memory without root; tuning requires explicit confirmation and administrator permissions.
- [x] No background daemon, hidden services, forced swap cycling, or cache drops.
- [x] Offline public release audit checks version and searches source for common credential forms.
- [ ] Public downloads and release page verified by an unauthenticated browser (requires repo to be public).
- [ ] Testing on independent Ubuntu, Fedora, Arch, openSUSE VMs, including older glibc baselines.
- [ ] Installation, removal, window-manager integration and root-authorized profile write/rollback tested on a disposable VM.
- [ ] Final GitHub Release with immutable tag and assets verified (requires GitHub Releases write path / CircleCI protected token).
- [ ] Owner decides to change repository visibility from private to public; do not change automatically.

## Supported target

Currently tested for Debian GNU/Linux 13 x86-64, native X11 or XWayland. The C++17 source may compile on additional distributions. Dynamic native packages are not universal Linux binaries. The `.deb` built here targets newer glibc/libstdc++, and may not install on older systems.

## Privacy guidance

Do not post screenshots containing usernames, hostname, SSH details, private tokens or private process names. The command `memboost --json` includes top process names; sanitize them before attaching logs. Security reports should omit private system information.
