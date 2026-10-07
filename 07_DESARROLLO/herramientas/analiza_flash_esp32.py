"""Analiza una imagen de flash ESP32 (solo lectura): particiones, otadata, descriptor de app y cadenas."""
import hashlib
import re
import struct
import sys

img = open(sys.argv[1], "rb").read()
TIPOS = {0: "app", 1: "data"}
SUB_APP = {0x00: "factory", 0x20: "test", **{0x10 + i: f"ota_{i}" for i in range(16)}}
SUB_DATA = {0x00: "ota", 0x01: "phy", 0x02: "nvs", 0x03: "coredump", 0x04: "nvs_keys", 0x05: "efuse", 0x80: "esphttpd", 0x81: "fat", 0x82: "spiffs", 0x83: "littlefs"}

print("== Cabecera bootloader @0x1000:", img[0x1000:0x1004].hex(), "(E9 = imagen válida)")
print("== Tabla de particiones @0x8000")
parts = []
for off in range(0x8000, 0x9000, 32):
    e = img[off:off + 32]
    if e[:2] == b"\xaa\x50":
        t, st, ofs, size = e[2], e[3], *struct.unpack("<II", e[4:12])
        name = e[12:28].split(b"\0")[0].decode(errors="replace")
        sub = (SUB_APP if t == 0 else SUB_DATA).get(st, hex(st))
        parts.append((name, t, st, ofs, size))
        print(f"  {name:10s} {TIPOS.get(t, t)!s:5s} {sub:9s} 0x{ofs:06x} 0x{size:06x} ({size // 1024} KiB)")
    elif e[:2] == b"\xeb\xeb":
        print("  MD5 tabla:", e[16:32].hex())
        break

for name, t, st, ofs, size in parts:
    if t == 1 and st == 0:
        for i in range(2):
            seq, = struct.unpack("<I", img[ofs + i * 0x1000: ofs + i * 0x1000 + 4])
            print(f"== otadata[{i}] seq = 0x{seq:08x}" + (" (vacío)" if seq == 0xFFFFFFFF else ""))

for name, t, st, ofs, size in parts:
    if t != 0:
        continue
    app = img[ofs:ofs + size]
    print(f"\n== App '{name}' @0x{ofs:06x}: primer byte {app[0]:#04x}", "(imagen)" if app[0] == 0xE9 else "(vacía/no imagen)")
    if app[0] != 0xE9:
        continue
    d = app[32:32 + 256]
    magic, = struct.unpack("<I", d[:4])
    if magic == 0xABCD5432:
        s = lambda a, b: d[a:b].split(b"\0")[0].decode(errors="replace")
        print("  version:", s(16, 48), "| proyecto:", s(48, 80), "| compilado:", s(96, 112), s(80, 96), "| IDF:", s(112, 144))
        print("  ELF SHA-256 (prefijo):", d[144:176].hex())
    else:
        print("  sin descriptor esp_app_desc")
    used = len(app.rstrip(b"\xff"))
    print(f"  bytes usados (aprox., sin 0xFF final): {used}")
    cad = set(re.findall(rb"[\x20-\x7e\xc0-\xff]{6,}", app))
    claves = [c for c in cad if re.search(rb"KART|CAN |CAN_|MPU|giroscop|Rel|STOP|carreg|WiFi|ssid|arduino-esp32|esp32", c, re.I)]
    for c in sorted(claves)[:60]:
        print("   ", c.decode("latin-1")[:110])

print("\n== Bloques de datos no vacíos (cada 64 KiB):")
for i in range(0, len(img), 0x10000):
    if img[i:i + 0x10000].strip(b"\xff"):
        print(f"  0x{i:06x}", end="")
print()
