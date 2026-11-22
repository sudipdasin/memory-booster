#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 MemBoost Pro contributors
"""Publish CI-verified release files using CircleCI, never GitHub Actions.

Requires an explicit tag on the tested commit, credentials in a protected
CircleCI context, and successful upstream CircleCI build-test-package job.
This script is intentionally not executed by default during ordinary pushes.
"""
import json
import mimetypes
import os
from pathlib import Path
import re
import sys
import urllib.error
import urllib.parse
import urllib.request

BASE = "https://api.github.com"
OWNER_RE = re.compile(r"^[A-Za-z0-9_.-]+$")
TAG_RE = re.compile(r"^v(\d+)\.(\d+)\.(\d+)$")


def fail(message):
    raise SystemExit("Release publication blocked: " + message)


def request(url, token, method="GET", data=None, content_type="application/vnd.github+json"):
    headers = {"Authorization": "Bearer " + token,
               "Accept": "application/vnd.github+json",
               "User-Agent": "memboost-circleci-release",
               "X-GitHub-Api-Version": "2022-11-28"}
    if data is not None:
        headers["Content-Type"] = content_type
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=45) as resp:
            body = resp.read()
            return json.loads(body.decode("utf-8")) if body else {}
    except urllib.error.HTTPError as exc:
        # Never print Authorization headers or credential material.
        if exc.code == 404:
            return None
        fail("GitHub API returned HTTP " + str(exc.code) + " for " + method)
    except urllib.error.URLError as exc:
        fail("GitHub API connection failed: " + str(exc.reason))


def verify_dist(directory):
    sums = directory / "SHA256SUMS.txt"
    if not sums.is_file():
        fail("expected dist/SHA256SUMS.txt missing")
    import hashlib
    files = []
    for line in sums.read_text(encoding="utf-8").splitlines():
        match = re.fullmatch(r"([0-9a-f]{64})  ([A-Za-z0-9_.+\-]+)", line)
        if not match:
            fail("invalid checksum manifest")
        digest, name = match.groups()
        path = directory / name
        if not path.is_file() or not path.resolve().is_relative_to(directory.resolve()):
            fail("missing or unsafe package name: " + name)
        if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            fail("SHA-256 mismatch for " + name)
        files.append(path)
    if not any(p.suffix == ".deb" for p in files) or not any(p.name.endswith(".tar.gz") for p in files):
        fail("DEB and TGZ artifacts are both required")
    return files + [sums]


def main():
    root = Path(__file__).resolve().parent.parent
    tag = os.getenv("CIRCLE_TAG", "")
    m = TAG_RE.fullmatch(tag)
    if not m:
        fail("must run from an explicit semantic-version tag")
    version = (root / "VERSION").read_text(encoding="utf-8").strip()
    if tag != "v" + version:
        fail("tag/version mismatch")
    owner = os.getenv("CIRCLE_PROJECT_USERNAME", "")
    repo = os.getenv("CIRCLE_PROJECT_REPONAME", "")
    if not OWNER_RE.fullmatch(owner) or not OWNER_RE.fullmatch(repo):
        fail("CircleCI repository identity missing or invalid")
    if owner != "sudipdasin" or repo != "memory-booster":
        fail("repository identity is not the authorized MemBoost Pro repository")
    token = os.getenv("GITHUB_TOKEN", "")
    if not token:
        fail("protected CircleCI GITHUB_TOKEN is not configured")
    files = verify_dist(root / "dist")
    note = root / "docs" / "releases" / (tag + ".md")
    if not note.is_file():
        fail("versioned release notes missing")
    api = f"{BASE}/repos/{owner}/{repo}/releases"
    release = request(api + "/tags/" + tag, token)
    if release is None:
        payload = {"tag_name": tag, "target_commitish": os.getenv("CIRCLE_SHA1", "main"),
                   "name": "MemBoost Pro " + tag, "body": note.read_text(encoding="utf-8"),
                   "draft": False, "prerelease": version.startswith("0.")}
        release = request(api, token, "POST", json.dumps(payload).encode("utf-8"))
    if not release or "upload_url" not in release:
        fail("release creation did not return an upload URL")
    existing = {asset.get("name") for asset in release.get("assets", [])}
    endpoint = release["upload_url"].split("{")[0]
    if not endpoint.startswith("https://uploads.github.com/"):
        fail("unexpected upload endpoint")
    for path in files:
        if path.name in existing:
            fail("asset already exists; refusing to silently overwrite: " + path.name)
        url = endpoint + "?" + urllib.parse.urlencode({"name": path.name})
        mime = "application/octet-stream"
        request(url, token, "POST", path.read_bytes(), mime)
        print("Published", path.name)
    print("Release created or populated:", release.get("html_url", "(URL unavailable)"))


if __name__ == "__main__":
    main()
