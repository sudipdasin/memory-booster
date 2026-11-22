# Permanent CI provider policy — CircleCI

**Recorded 2026-11-22.** Per project owner: GitHub Actions usage is exhausted and a billing restriction prevents jobs executing. Use **CircleCI** as the exclusive active CI provider for tests, security checks, packaging and authorized releases. GitHub remains source control and PR/release storage. Do not upgrade billing, re-enable GitHub Actions or switch providers without owner authorization.

Canonical policy: https://github.com/sudipdasin/CI-Master-Control/blob/main/docs/STANDING_CI_PROVIDER_POLICY.md

**MemBoost v0.1.1:** The repository contains C++ application code and CircleCI automated unit, CLI, virtual-X11 and packaging checks. CircleCI enrollment is **observed** via a reported `ci/circleci: build-test-package` status. The first observed job reported a failure; a reproducible local packaging-check issue has been corrected, but the new job must pass before CI is certified. Local Linux build/tests are separately documented.
