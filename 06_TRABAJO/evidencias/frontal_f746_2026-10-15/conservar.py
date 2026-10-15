"""ENS-0019: manifiesto SHA-256 de evidencias y respaldos locales (no copia nada)."""
import hashlib, json, time
from pathlib import Path
RAIZ = Path(__file__).resolve().parents[3]
EVID = Path(__file__).resolve().parent
RESP = RAIZ / "07_DESARROLLO/respaldos/stm32_f746/2026-10-15"
def h(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def tabla(base):
    return {p.relative_to(RAIZ).as_posix(): {"bytes": p.stat().st_size, "sha256": h(p)}
            for p in sorted(base.rglob("*")) if p.is_file() and p.name != "manifest.json"}
fuentes = ["07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.hex",
           "07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.elf",
           "07_DESARROLLO/firmware/stm32_frontal/tools/observar_swd.py",
           "07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c",
           "07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_display.c",
           "07_DESARROLLO/firmware/stm32_frontal/Core/Inc/kart_frontend.h"]
m = {"generado_local": time.strftime("%Y-%m-%dT%H:%M:%S"), "ensayo": "ENS-0019",
     "fuentes_y_binario_grabado": {f: h(RAIZ / f) for f in fuentes},
     "evidencias": tabla(EVID), "respaldos_locales": tabla(RESP)}
(EVID / "manifest.json").write_text(json.dumps(m, indent=1, ensure_ascii=False), encoding="utf-8")
print(len(m["evidencias"]), "evidencias,", len(m["respaldos_locales"]), "respaldos")
