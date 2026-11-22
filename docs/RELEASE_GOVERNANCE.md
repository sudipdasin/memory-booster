# Governance — canonical MemBoost Pro repository

**Owner:** `@sudipdasin`. All incoming improvements to the canonical repository require a pull request and your approval before merging. GPL gives downstream rights to fork and distribute; it does not grant anyone write privileges to this repository.

## GitHub settings the owner must activate

Use `Settings → Rules → Rulesets` to create active branch rules for `main` and `release/*` (or use branch protection if rulesets unavailable). Set:

- Require pull request before merging; require approval from a code owner (`.github/CODEOWNERS` contains `* @sudipdasin`), dismiss stale approvals, and require the most recent reviewable push to be approved by a person other than its pusher where possible.
- Require **all three exact CircleCI checks**: `ci/circleci: build-test-package`, `ci/circleci: headless-sanitizers`, and `ci/circleci: ubuntu-2404-vm-install`. Never replace them with GitHub Actions.
- Block force pushes and deletions; apply rules to admins and disable bypasses you do not want; avoid granting broad merge or direct-push privileges.
- Keep auto-merge disabled and use the CircleCI `hold-release` manual release approval step, with restricted credentials in a CircleCI context.

**Solo maintainer caveat:** GitHub does not permit PR authors to approve their own PRs. For assistant-authored commits authenticated as your user, required codeowner approval may require an independent trusted maintainer or a deliberate owner-only bypass exception. Choose the policy deliberately, test with a sample PR and do not silently circumvent it.

## Merging order

Historical public-launch PRs #1 (v2), #2 (v3) and #3 (GPL v3.0.1) were merged into `main` under the owner's final release-preparation authorization. For **all future development**, require separate owner approval for each PR before merging. This policy document does **not** itself configure GitHub settings.

## Public launch gate

Before visibility becomes public, inspect repository history, assets, licenses and all older versioned downloads. Reconfirm checksums and the actual tag/branch commit. A public repository exposes history and all public branches, not just `main`. Publishing a GitHub Release requires a separate approved tag and protected CircleCI context.
