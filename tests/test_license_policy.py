#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 MemBoost Pro contributors
"""Offline regression gate for public-facing GPL license consistency."""
from pathlib import Path
import re
ROOT = Path(__file__).resolve().parents[1]
license_file = ROOT / 'LICENSE'
assert license_file.is_file()
text = license_file.read_text(encoding='utf-8')
assert 'GNU GENERAL PUBLIC LICENSE' in text
assert 'Version 3, 29 June 2007' in text
assert 'TERMS AND CONDITIONS' in text and 'END OF TERMS AND CONDITIONS' in text
assert len(text) > 34000, 'must include complete official GPL-3 text'
version=(ROOT/'VERSION').read_text().strip()
assert re.fullmatch(r'\d+\.\d+\.\d+',version)
assert f'project(MemBoostPro VERSION {version}' in (ROOT/'CMakeLists.txt').read_text()
assert 'CPACK_PACKAGE_LICENSE "GPL-3.0-or-later"' in (ROOT/'CMakeLists.txt').read_text()
assert 'GNU General Public License' in (ROOT/'README.md').read_text()
assert 'MIT' in (ROOT/'COPYRIGHT.md').read_text(), 'historical MIT terms must be disclosed'
assert 'MIT License' in (ROOT/'LICENSES/MIT-legacy.txt').read_text()
assert 'Permission is hereby granted' in (ROOT/'LICENSES/MIT-legacy.txt').read_text()
assert (ROOT/'.github/CODEOWNERS').read_text().strip().endswith('* @sudipdasin')
for directory, extensions in [('src', {'.cpp', '.hpp'}), ('tests', {'.cpp', '.py'}), ('scripts', {'.py'})]:
    for f in (ROOT/directory).rglob('*'):
        if f.is_file() and f.suffix in extensions:
            assert 'SPDX-License-Identifier: GPL-3.0-or-later' in f.read_text()[:220], f
for f in ['docs/LICENSE_OPTIONS.md', 'docs/PUBLIC_READINESS.md', 'docs/EDITION_POLICY.md']:
    contents=(ROOT/f).read_text()
    assert 'GPL-3.0-or-later' in contents, f
assert not (ROOT/'.github/workflows').exists(), 'CircleCI-only policy'
print('License-publication policy checks: PASS')
