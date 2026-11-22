# Contributing to MemBoost Pro

Contributions are welcome through **pull requests only**. The official `main` and release branches are intended to be protected, and the repository owner (`@sudipdasin`) must review external changes before merge. Do not push directly to `main`, and do not publish a public release on the project's behalf without the owner's authorization. See [review controls](docs/RELEASE_GOVERNANCE.md) and [.github/CODEOWNERS](.github/CODEOWNERS).

Include unit or CLI regression tests for changes, use CircleCI rather than GitHub Actions, and maintain nonprivileged default behavior. Reproduce bugs with MemBoost version, distribution/kernel, X11/XWayland/headless session, sanitized output, and clear reproduction steps. Avoid posting API keys, usernames, network topology or unredacted process names.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 scripts/public_release_audit.py
```

Licensing is currently MIT; a GPL-3.0-or-later proposal is documented but requires the owner's express agreement. Third-party contributors should not make unilateral licensing changes.
