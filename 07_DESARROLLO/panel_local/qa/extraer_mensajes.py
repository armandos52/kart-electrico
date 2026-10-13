"""Extrae de server.py y calibracion.py los mensajes en español que llegan a la interfaz.

Salida: JSON con las etiquetas/unidades de SIGNALS y los textos de ValueError, eventos, mensajes de la guía de
calibración, motivos de interrupción y errores HTTP. qa/check-idiomas.mjs comprueba que cada uno tenga traducción.
Solo lectura: no abre puertos ni arranca el servicio.
"""
import ast
import json
import sys
from pathlib import Path

BASE = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(BASE))
import server  # noqa: E402  (importar no abre puertos)


def flatten(node):
    if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Add):
        return flatten(node.left) + flatten(node.right)
    return [node]


def text_of(node):
    """Constante de texto, o concatenación con una parte variable (que se sustituye por 'x')."""
    parts = flatten(node)
    if not any(isinstance(p, ast.Constant) and isinstance(p.value, str) for p in parts):
        return None
    return "".join(p.value if isinstance(p, ast.Constant) and isinstance(p.value, str) else "x" for p in parts)


def collect(path):
    tree = ast.parse(path.read_text(encoding="utf-8"))
    found = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            name = node.func.id if isinstance(node.func, ast.Name) else node.func.attr if isinstance(node.func, ast.Attribute) else ""
            if name in {"ValueError", "interrupt", "event"} and node.args:
                found.add(text_of(node.args[0]))
            if name in {"finish", "pause_step"} and len(node.args) > 1:
                found.add(text_of(node.args[1]))
            for keyword in node.keywords:
                if keyword.arg in {"message", "reason", "detail"}:
                    found.add(text_of(keyword.value))
        if isinstance(node, ast.Dict):
            for key, value in zip(node.keys, node.values):
                if isinstance(key, ast.Constant) and key.value in {"error", "message", "reason", "detail"}:
                    found.add(text_of(value))
        if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id in {"reason", "message"} for t in node.targets):
            found.add(text_of(node.value))
        if isinstance(node, ast.Assign) and any(isinstance(t, ast.Attribute) and t.attr == "error" for t in node.targets):
            found.add(text_of(node.value))
        if isinstance(node, ast.Return) and node.value is not None and path.name == "calibracion.py":
            found.add(text_of(node.value))
    return found


messages = set()
for name in ("server.py", "calibracion.py"):
    messages |= collect(BASE / name)
messages.discard(None)
signals = set()
for label, unit, *_ in server.SIGNALS.values():
    signals.add(label)
    if unit and unit.isalpha() and len(unit) > 3:   # solo unidades que son palabras (p. ej. «cuentas»)
        signals.add(unit)
signals.add("MPU: lectura reciente y calibrada")  # etiqueta alternativa con firmware v1.3
enums = sorted({value for definition in server.SIGNALS.values() if isinstance(definition[2], set) for value in definition[2]})
payload = {"signals": sorted(signals), "messages": sorted(m for m in messages if any(c.isalpha() for c in m)),
           "enums": enums, "signal_keys": sorted(server.SIGNALS)}
sys.stdout.buffer.write(json.dumps(payload, ensure_ascii=False).encode("utf-8"))   # bytes: independiente de la consola
