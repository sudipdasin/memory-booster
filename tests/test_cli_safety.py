#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 MemBoost Pro contributors
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
    assert run('--diagnose').returncode == 0
    signals = run('--signals')
    assert signals.returncode == 0 and 'cumulative counters' in signals.stdout
    watch = run('--watch', '--samples', '2', '--interval-ms', '200')
    assert watch.returncode == 0 and 'Sample 2/2' in watch.stdout
    for invalid in (('--watch', '--samples', '0'), ('--watch', '--samples', '61'),
                    ('--watch', '--interval-ms', '100'), ('--watch', '--samples', 'abc'),
                    ('--watch', '--samples'), ('--watch', '--bogus', '1')):
        assert run(*invalid).returncode != 0, f'Expected rejection: {invalid}' 
    track = run('--track', '--samples', '2', '--interval-ms', '200', '--format', 'jsonl')
    assert track.returncode == 0, track.stderr
    records = [json.loads(line) for line in track.stdout.splitlines()]
    assert len(records) == 2 and records[0]['sample'] == 1 and records[1]['sample'] == 2
    assert records[0]['telemetry_schema'] == 1 and records[1]['interval']['seconds'] > 0
    assert records[1]['snapshot']['schema_version'] == 3
    for invalid in (('--track', '--samples', '1'), ('--track', '--samples', '61'),
                    ('--track', '--interval-ms', '100'), ('--track', '--samples', 'bad'),
                    ('--track', '--samples', '-2'), ('--track', '--samples', '1'*100),
                    ('--track', '--format', 'yaml'), ('--track', '--unknown', '3'),
                    ('--track', '--interval-ms'), ('--signals', '--yes')):
        assert run(*invalid).returncode != 0, f'Track accepted {invalid}'
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
        assert data['schema_version'] == 3
        assert data['diagnosis']['level'] in ('normal', 'elevated', 'high', 'unknown')
        assert isinstance(data['cgroup']['available'], bool)
        assert isinstance(data['vmstat']['available'], bool)
        assert isinstance(data['cgroup_events']['available'], bool)
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
