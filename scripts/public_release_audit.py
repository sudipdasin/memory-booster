#!/usr/bin/env python3
"""Offline public-exposure audit for the exact repository checkout; never uploads files."""
import argparse
import hashlib
from pathlib import Path
import re
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
PATTERNS={
    'PEM private key':re.compile(rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----'),
    'GitHub access token':re.compile(rb'(?:gh[pousr]_[A-Za-z0-9_]{30,}|github_pat_[A-Za-z0-9_]{30,})'),
    'AWS access ID':re.compile(rb'AKIA[0-9A-Z]{16}'),
    'OpenAI credential':re.compile(rb'sk-(?:proj-)?[A-Za-z0-9_-]{24,}'),
}

def sha256(path):
    h=hashlib.sha256()
    with path.open('rb') as fp:
        for block in iter(lambda:fp.read(128*1024), b''):
            h.update(block)
    return h.hexdigest()

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--artifacts',type=Path,help='directory with package artifacts for exact-version verification')
    args=ap.parse_args()
    version=(ROOT/'VERSION').read_text().strip()
    cmake=(ROOT/'CMakeLists.txt').read_text()
    assert re.fullmatch(r'\d+\.\d+\.\d+',version), 'invalid release version'
    assert f'project(MemBoostPro VERSION {version} ' in cmake, 'version mismatch'
    assert (ROOT/'LICENSE').is_file(), 'source publication requires an explicit license'
    assert (ROOT/'.circleci/config.yml').is_file(), 'CircleCI policy missing'
    assert not (ROOT/'.github/workflows').exists(), 'GitHub Actions forbidden by CI policy'
    candidates=[]
    for d in ('src','tests','scripts','packaging','docs'):
        candidates.extend((ROOT/d).rglob('*'))
    candidates.extend(ROOT.glob('*'))
    candidates.extend((ROOT/'.circleci').rglob('*'))
    for file in candidates:
        if not file.is_file() or file.stat().st_size>2_000_000:continue
        if any(part.startswith('build-') for part in file.parts):continue
        b=file.read_bytes()
        if b'\0' in b:continue
        for label,rx in PATTERNS.items():
            if rx.search(b):raise AssertionError(f'potential {label} in {file.relative_to(ROOT)}')
    if args.artifacts:
        dest=args.artifacts.resolve()
        manifest=dest/'SHA256SUMS.txt'
        assert manifest.is_file(), 'checksums missing'
        names=[]
        for line in manifest.read_text().splitlines():
            m=re.fullmatch(r'([a-f0-9]{64})  ([\w.+-]+)',line)
            assert m, 'malformed manifest entry'
            digest,name=m.groups()
            assert name not in names, 'duplicate manifest entry'
            assert (dest/name).is_file(),f'missing {name}'
            assert sha256(dest/name)==digest,f'checksum mismatch: {name}'
            names.append(name)
        assert any(n.endswith('.deb') for n in names) and any(n.endswith('.tar.gz') for n in names)
        for n in names:assert version in n,'package version is stale: '+n
    print(f'Public-release audit passed for version {version}; no credential patterns in source files')

if __name__=='__main__':
    try:main()
    except (OSError,AssertionError) as exc:
        print('PUBLIC RELEASE BLOCKED:',exc,file=sys.stderr)
        raise SystemExit(1)
