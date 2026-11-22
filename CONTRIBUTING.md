# Contributing

To reproduce a defect, include the MemBoost version, distribution/kernel, session type (X11/XWayland/headless), sanitized `memboost --self-test` output, and exact reproduction steps. Avoid submitting credentials or unredacted process lists. Run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 scripts/public_release_audit.py
```

Only CircleCI is authorized for hosted CI; do not add GitHub Actions workflows. Contributions should include regression tests, preserve nonprivileged defaults, and avoid unrequested persistent system changes.
