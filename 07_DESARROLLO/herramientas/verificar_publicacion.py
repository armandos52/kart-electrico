"""Comprueba enlaces Markdown publicados, tamaño y nombres de archivos privados.

Uso desde cualquier carpeta: python verificar_publicacion.py [--json salida.json]
Examina solo archivos versionados/candidatos no ignorados, nunca todo el archivo histórico.
No comprueba URLs externas ni afirma que una búsqueda automática descarte todo secreto.
Las copias inmutables de evidencias conservan su contenido y sus enlaces históricos.
"""
import argparse
import json
import re
import subprocess
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[2]


def candidates():
    raw = subprocess.check_output(
        ['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'], cwd=ROOT)
    return sorted({p for p in raw.decode('utf-8').split('\0') if p and (ROOT / p).is_file()})


def check():
    files = candidates()
    published = set(files)
    errors = []
    links = 0
    local_only = 0
    for name in files:
        p = ROOT / name
        if p.stat().st_size > 50 * 1024 * 1024:
            errors.append({'file': name, 'reason': 'archivo mayor de 50 MiB'})
        if p.name == 'secretos.h' or p.name == '.env' or (
            name.startswith('07_DESARROLLO/respaldos/') and p.suffix in {'.bin', '.elf', '.hex'}):
            errors.append({'file': name, 'reason': 'archivo privado o imagen de memoria'})
        if p.suffix != '.md' or '/evidencias/' in name or '/vendor/' in name or '/lib/mcp_can/' in name:
            continue
        source = p.read_text(encoding='utf-8-sig')
        source = re.sub(r'```.*?```', '', source, flags=re.S)
        if re.search(r'\[\[[^\]]+\]\]', source):
            errors.append({'file': name, 'reason': 'enlace exclusivo de Obsidian'})
        for match in re.finditer(r'!?\[[^\]\n]*\]\((<[^>]+>|[^\s)]+)(?:\s+"[^"]*")?\)', source):
            dest = match[1].strip('<>')
            if urlsplit(dest).scheme or dest.startswith('#'):
                continue
            links += 1
            target = (p.parent / unquote(dest.split('#')[0].split('?')[0])).resolve()
            try:
                rel = target.relative_to(ROOT).as_posix()
            except ValueError:
                rel = ''
            private = rel.startswith(('02_ORIGINALES/entrega_2026-09-28/',
                                      '02_ORIGINALES/fotos_kart_2026-10-08/',
                                      '08_INDICE_GENERADO/',
                                      '07_DESARROLLO/panel_local/sesiones/')) or (
                rel.startswith('07_DESARROLLO/respaldos/') and target.suffix in {'.bin', '.elf', '.hex'})
            if private and 'solo local' in match[0]:
                local_only += 1
                continue
            if rel not in published and not any(f.startswith(rel.rstrip('/') + '/') for f in published if rel):
                errors.append({'file': name, 'target': dest, 'reason': 'destino no publicado'})
    return {'files': len(files), 'local_links': links, 'intentional_local_only': local_only, 'errors': errors,
            'scope': 'Enlaces a archivos, no anclas ni URLs externas; excluye copias de evidencia y dependencias.'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--json', type=Path)
    args = parser.parse_args()
    result = check()
    if args.json:
        args.json.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result, ensure_ascii=False, indent=2))
    raise SystemExit(bool(result['errors']))
