# Review, ownership and publication controls

MemBoost Pro's official repository is governed by its owner. Changes from outside contributors should be proposed as pull requests and must be deliberately reviewed by `@sudipdasin` before merging.

## Repository owner configuration required in GitHub

1. Open **Settings → Rules → Rulesets** (or Settings → Branches → Branch protection) and create an active rule targeting `main` and `release/*`.
2. Require a pull request before merging; require approving reviews, request reviews from CODEOWNERS, dismiss stale approvals on further commits, and require passing `ci/circleci` status checks (use the actual status names displayed on a successful PR).
3. Disable force pushes, branch deletion and any unwanted admin bypass; turn off auto-merge. Limit direct-push access and ensure actions/bots have minimum required permissions.
4. Keep GitHub Actions disabled per `docs/CI_PROVIDER_POLICY.md`. CI must run in CircleCI, which is not a substitute for an owner's review.
5. Review all release assets and approve CircleCI's manual `hold-release` gate before any GitHub Releases publishing job executes. The publishing job also needs a protected, minimal-scope credential.
6. Check settings with a test PR made by a contributor before changing repository visibility.

**Technical limitation:** GitHub does not allow a PR author to approve their own PR. A solo maintainer who authors changes must instead obtain an independent review or use a deliberately configured owner exception. CODEOWNERS alone does not protect a branch. Repository owners may be able to bypass protections depending on the configured ruleset.

Anyone may fork, modify or redistribute legally under the open-source license, but **that does not grant them write access to the canonical `sudipdasin/memory-booster` repository**.

## Release conditions

Use a versioned, protected release branch or a signed annotated tag on a tested commit. For a public release confirm: (a) explicit owner approval of the exact source diff and license, (b) complete green CircleCI build, sanitizer and independent Linux VM installation jobs, (c) exact archive hashes, (d) a user-installable release artifact and documentation, and (e) public, unauthenticated download checks after making the repo public. Do not claim cross-distro support beyond what was actually tested.
