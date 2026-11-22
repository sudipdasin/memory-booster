#!/usr/bin/env python3
"""Nonprivileged CLI integration; never runs --yes against a known profile."""
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import subprocess
import sys

EXE = sys.argv[1]
SYSCTL = [Path('/proc/sys/vm/swappiness'), Path('/proc/sys/vm/vfs_cache_pressure')]


def values():
    return [p.read_text() if p.is_file() else None for p in SYSCTL]


def run(*args):
    return subprocess.run([EXE, *args], capture_output=True, text=True, timeout=10)


def main():
    before = values()
    assert run('--help').returncode == 0
    assert run('--version').returncode == 0
    assert run('--status').returncode == 0
    self_test = run('--self-test')
    assert self_test.returncode == 0 and 'self-test: PASS' in self_test.stdout
    for p in ('balanced', 'responsive', 'compressed-swap'):
        r = run('--apply', p, '--dry-run')
        assert r.returncode == 0 and 'Dry run:' in r.stdout
        r = run('--apply', p)
        assert r.returncode != 0 and 'Refusing changes' in r.stderr
    for params in (('--unknown',), ('--status', '--yes'), ('--apply', 'INVALID', '--yes'), ('--apply', 'balanced', '--undefined')):
        assert run(*params).returncode != 0
    def worker(index):
        data = json.loads(run('--json').stdout)
        assert data['valid'] is True
        assert 0 <= data['ram_available_kib'] <= data['ram_total_kib']
        assert 0 <= data['swap_free_kib'] <= data['swap_total_kib']
        assert isinstance(data['processes'], list)
        assert index >= 0
    with ThreadPoolExecutor(max_workers=8) as ex:
        list(ex.map(worker, range(32)))
    assert values() == before, 'A read-only or rejected CLI call modified host sysctls'
    print('CLI safety: 32 parallel JSON reads, profile dry-runs, confirmation refusals, host state unchanged')


if __name__ == '__main__':
    main()
