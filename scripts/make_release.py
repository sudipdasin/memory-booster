#!/usr/bin/env python3
"""Create reproducible source archive and locally tested Linux release bundle.

This script does not push, tag, publish, alter permissions or change kernel state.
"""
from pathlib import Path
import argparse
import gzip
import hashlib
import os
import subprocess
import tarfile
import zipfile

ROOT=Path(__file__).resolve().parents[1]
SOURCE_FILES=['.circleci/config.yml','.gitignore','CHANGELOG.md','CMakeLists.txt',
              'LICENSE','README.md','VERSION','SECURITY.md','CONTRIBUTING.md']
SOURCE_DIRS=['src','tests','packaging','scripts','docs','assets']

def run(*args):
    subprocess.run(args, cwd=ROOT, check=True)

def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as stream:
        for data in iter(lambda:stream.read(1024*1024),b''):h.update(data)
    return h.hexdigest()

def make_tar(files, dest, prefix):
    # Fixed ownership/time and deterministic gzip header for reproducible source snapshots.
    with dest.open('wb') as raw:
        with gzip.GzipFile(filename='',mode='wb',mtime=0,fileobj=raw) as zipped:
            with tarfile.open(mode='w',fileobj=zipped) as archive:
                for file in sorted(files):
                    relative=file.relative_to(ROOT)
                    name=str(Path(prefix)/relative)
                    item=archive.gettarinfo(str(file),arcname=name)
                    item.uid=item.gid=0;item.uname=item.gname='';item.mtime=0
                    with file.open('rb') as f:archive.addfile(item,f)

def add_zip(z, file):
    zi=zipfile.ZipInfo(file.name,date_time=(2026,11,22,0,0,0))
    zi.compress_type=zipfile.ZIP_DEFLATED
    zi.external_attr=0o644<<16
    z.writestr(zi,file.read_bytes())

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--build-dir',default='build-stable')
    args=p.parse_args()
    version=(ROOT/'VERSION').read_text().strip()
    assert version.count('.')==2
    build=ROOT/args.build_dir
    run('cmake','-S','.','-B',str(build),'-DCMAKE_BUILD_TYPE=Release','-DMEMBOOST_BUILD_GUI=ON')
    run('cmake','--build',str(build),'--parallel','4')
    run('ctest','--test-dir',str(build),'--output-on-failure')
    out=subprocess.check_output([str(build/'memboost'),'--version'],text=True).strip()
    assert out==f'MemBoost Pro {version}',(out,version)
    run('cpack','--config',str(build/'CPackConfig.cmake'),'-G','DEB','-B',str(build))
    run('cpack','--config',str(build/'CPackConfig.cmake'),'-G','TGZ','-B',str(build))
    dest=ROOT/'downloads'/f'v{version}'
    dest.mkdir(parents=True,exist_ok=True)
    base=f'MemBoost-Pro-{version}'
    products=[]
    for suffix,ext in [('deb','deb'),('tar.gz','tar.gz')]:
        original=build/f'memboost-pro-{version}-Linux.{ext}'
        assert original.is_file(),str(original)
        target=dest/f'{base}-linux-amd64.{ext}'
        target.write_bytes(original.read_bytes())
        products.append(target)
    source=dest/f'{base}-source.tar.gz'
    sources=[ROOT/name for name in SOURCE_FILES]
    for folder in SOURCE_DIRS:
        sources.extend(f for f in (ROOT/folder).rglob('*') if f.is_file() and '__pycache__' not in f.parts and f.suffix!='.pyc')
    make_tar(sources,source,f'memboost-pro-{version}')
    products.append(source)
    lines=[f'{digest(f)}  {f.name}' for f in products]
    (dest/'SHA256SUMS.txt').write_text('\n'.join(lines)+'\n')
    notes=ROOT/'docs'/'releases'/f'v{version}.md'
    assert notes.is_file(),str(notes)
    (dest/'RELEASE-NOTES.md').write_bytes(notes.read_bytes())
    bundle=dest/f'{base}-download-bundle.zip'
    with zipfile.ZipFile(bundle,'w') as z:
        for f in products+[dest/'SHA256SUMS.txt',dest/'RELEASE-NOTES.md',ROOT/'LICENSE']:
            add_zip(z,f)
    print('Release bundle completed:',dest)
    for f in products+[bundle]:print(f.name,f.stat().st_size,'bytes SHA256',digest(f))

if __name__=='__main__':
    main()
