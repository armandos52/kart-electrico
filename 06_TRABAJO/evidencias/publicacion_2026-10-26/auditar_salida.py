"""Inspección acotada del contenido Git a publicar; nunca imprime valores secretos.

Usa origin/main como base. Incluye todos los blobs nuevos del historial y los ZIP/DOCX.
La contraseña local, si existe, se compara en memoria sin guardarla.
"""
from pathlib import Path
import io
import json
import re
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[3]


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def main():
    secret_file = ROOT / '07_DESARROLLO/firmware/esp32_trasero/include/secretos.h'
    known = []
    if secret_file.exists():
        known = [v.encode() for _, v in re.findall(
            r'#define\s+(\w*(?:PASS|PASSWORD|CLAVE)\w*)\s+"([^"]+)"',
            secret_file.read_text(), re.I) if len(v) >= 8]
    patterns = [
        ('token', re.compile(rb'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{50,}|sk-[A-Za-z0-9_-]{32,}|AKIA[0-9A-Z]{16})')),
        ('private_key', re.compile(rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----')),
    ]
    findings = []
    objects = git('rev-list', '--objects', 'origin/main..HEAD').decode().splitlines()
    entries = dict(line.split(' ', 1) if ' ' in line else (line, '') for line in objects)
    batch = subprocess.Popen(['git', 'cat-file', '--batch'], cwd=ROOT,
                             stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    count = 0
    size = 0

    def scan(name, data):
        nonlocal count, size
        count += 1
        size += len(data)
        if any(value in data for value in known):
            findings.append({'path': name, 'reason': 'coincidencia con clave local'})
        for label, pattern in patterns:
            if pattern.search(data):
                findings.append({'path': name, 'reason': label})
        if data.startswith(b'PK\x03\x04'):
            try:
                with zipfile.ZipFile(io.BytesIO(data)) as archive:
                    for item in archive.infolist():
                        if not item.is_dir() and item.file_size < 10_000_000:
                            scan(name + '!' + item.filename, archive.read(item))
            except zipfile.BadZipFile:
                findings.append({'path': name, 'reason': 'ZIP no legible'})

    try:
        for oid, name in entries.items():
            batch.stdin.write((oid + '\n').encode())
            batch.stdin.flush()
            header = batch.stdout.readline().split()
            length = int(header[2])
            data = batch.stdout.read(length)
            batch.stdout.read(1)
            if header[1] == b'blob':
                scan(name + '@' + oid, data)
    finally:
        batch.stdin.close()
        batch.wait()
    paths = git('ls-tree', '-r', '--name-only', 'HEAD').decode().splitlines()
    forbidden = [p for p in paths if Path(p).name in {'secretos.h', '.env'} or (
        p.startswith('07_DESARROLLO/respaldos/') and Path(p).suffix in {'.bin', '.elf', '.hex'})]
    report = {
        'head': git('rev-parse', 'HEAD').decode().strip(),
        'base': git('rev-parse', 'origin/main').decode().strip(),
        'remote': git('remote', 'get-url', 'origin').decode().strip(),
        'scope': 'Blobs del historial origin/main..HEAD; tokens comunes, claves privadas y coincidencia con clave Wi-Fi local. Incluye contenido ZIP/DOCX menor de 10 MB por entrada. No auditoría exhaustiva.',
        'blobs_and_archive_entries': count, 'bytes_scanned': size,
        'tracked_files': len(paths), 'forbidden_files': forbidden, 'findings': findings,
        'backup_paths': [p for p in paths if p.startswith('07_DESARROLLO/respaldos/')],
    }
    output = Path(__file__).with_name('auditoria_commit.json')
    output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return bool(forbidden or findings)


if __name__ == '__main__':
    raise SystemExit(main())
