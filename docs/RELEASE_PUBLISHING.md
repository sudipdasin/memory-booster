# GPL-3.0-or-later public release procedure (CircleCI only)

1. Review and approve the v2 PR, the v3 PR and the GPL v3.0.1 PR in that order. Require all three CircleCI jobs on the exact head commit: `ci/circleci: build-test-package`, `ci/circleci: headless-sanitizers`, `ci/circleci: ubuntu-2404-vm-install`.
2. Enable enforceable GitHub rulesets before merging (`main`, `release/*`); explicitly deny unwanted admin bypasses, direct pushes, force pushes and deletion. An author may not approve their own PR. CODEOWNERS is not enforced without required code-owner reviews.
3. Verify LICENSE (GPL version 3 full text), COPYRIGHT.md, SPDX notices, source/build reproducibility and current GPL-bearing archives. Retain prior MIT artifacts and notices without relabeling them. Check all branches, history and included images/assets for any private information before exposing.
4. Merge owner-approved PRs, validate the final version and checksums on `main`, and run CircleCI on the exact merged commit. **Only then** change GitHub repository visibility to public.
5. Verify anonymous downloads under `downloads/v3.0.1/` from a logged-out session; this is repository-hosted distribution, **not** an official GitHub Releases entry.
6. Optional official GitHub Release: configure a narrowly restricted CircleCI `memboost-release` context with a scoped GitHub token (Contents write only), confirm authorization and manual hold, create tag `v3.0.1` on the verified final commit, approve `hold-release`, then confirm the attached GitHub Release assets. Never commit a token or put it in a PR comment.

A GPL program may be distributed commercially; independent forks do not require permission to publish, but the official repository requires owner review for merges and official releases.

## Publishing v3.2.0 as a **real GitHub Release**

The repository-hosted `downloads/v3.2.0/` files are **not** a GitHub Release. Once the owner has reviewed and merged the v3.2.0 PR and verified all three CircleCI checks on the merge commit:

1. Visit https://github.com/sudipdasin/memory-booster/releases/new, create a new tag `v3.2.0` from the verified `main` branch commit, and title it `MemBoost Pro Community v3.2.0`.
2. Use `docs/releases/v3.2.0.md` as notes and attach the tested DEB, Linux TGZ, source TGZ, full ZIP, and SHA256SUMS file from `downloads/v3.2.0/`. Set as latest and publish.
3. Alternatively, if the restricted CircleCI context `memboost-release` already contains a **scoped Contents-write token**, approve its tag-triggered `hold-release` job after successful required jobs. Never commit or paste tokens into chat.
4. Verify the **GitHub Releases** page lists the tag and downloadable attachments, compare SHA-256, and test unauthenticated downloads and installed app.

Neither a PR nor a repository-hosted ZIP creates a GitHub Release entry. **No release should be described as published until GitHub confirms it.**
