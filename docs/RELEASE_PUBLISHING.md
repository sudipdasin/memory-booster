# GPL-3.0-or-later public release procedure (CircleCI only)

1. Review and approve the v2 PR, the v3 PR and the GPL v3.0.1 PR in that order. Require all three CircleCI jobs on the exact head commit: `ci/circleci: build-test-package`, `ci/circleci: headless-sanitizers`, `ci/circleci: ubuntu-2404-vm-install`.
2. Enable enforceable GitHub rulesets before merging (`main`, `release/*`); explicitly deny unwanted admin bypasses, direct pushes, force pushes and deletion. An author may not approve their own PR. CODEOWNERS is not enforced without required code-owner reviews.
3. Verify LICENSE (GPL version 3 full text), COPYRIGHT.md, SPDX notices, source/build reproducibility and current GPL-bearing archives. Retain prior MIT artifacts and notices without relabeling them. Check all branches, history and included images/assets for any private information before exposing.
4. Merge owner-approved PRs, validate the final version and checksums on `main`, and run CircleCI on the exact merged commit. **Only then** change GitHub repository visibility to public.
5. Verify anonymous downloads under `downloads/v3.0.1/` from a logged-out session; this is repository-hosted distribution, **not** an official GitHub Releases entry.
6. Optional official GitHub Release: configure a narrowly restricted CircleCI `memboost-release` context with a scoped GitHub token (Contents write only), confirm authorization and manual hold, create tag `v3.0.1` on the verified final commit, approve `hold-release`, then confirm the attached GitHub Release assets. Never commit a token or put it in a PR comment.

A GPL program may be distributed commercially; independent forks do not require permission to publish, but the official repository requires owner review for merges and official releases.
