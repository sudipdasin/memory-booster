# MemBoost Pro release publishing (CircleCI only)

1. Keep GitHub Actions disabled; CircleCI is the exclusive CI provider due to billing restrictions.
2. Run the local and remote full CI verification jobs on the exact release commit, including `build-test-package` and `headless-sanitizers`. Both must pass.
3. Review MIT LICENSE, bundled artwork, source archive, README, security/privacy checklist and Debian dependencies before public visibility is enabled. Changing the visibility is a separate, potentially irreversible exposure decision.
4. If you want assets in GitHub's official **Releases** section, create a *restricted CircleCI context* named `memboost-release` with a protected `GITHUB_TOKEN` that is permitted to publish releases to this repository (fine-grained access: Contents read/write only). Configure CircleCI context security restrictions so forks, arbitrary contributors and untrusted tags cannot access this token. **Never commit tokens or paste them into public issues.**
5. Create the exact version tag (e.g. `v1.0.0`) on the verified release commit. The tag-only `publish-github-release` CircleCI job depends on both verification jobs and rejects version mismatches, missing credentials and checksum failures. Do not create the tag before the protected context is ready; it will otherwise fail to publish. The publisher script never silently overwrites existing assets.
6. Confirm public GitHub Releases assets and file hashes from an unauthenticated browser after making the repository public. This step cannot be verified while the repository is private.

**Fallback:** Versioned checksummed `downloads/v1.0.0/` are already included in the repository and linked in the main README. They will be available to everyone when the repository owner deliberately switches visibility to public, even if official GitHub Releases attachment is not set up. This fallback does not create an official release or provide cryptographic signature verification.
