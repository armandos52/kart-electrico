"""Ensayo Wi-Fi de la telemetría v1.1 (solo lectura; nunca transmite al ESP32).

Fases: conectar a kart-esp32 -> TCP 30 s -> cerrar y reconectar TCP -> cortar Wi-Fi del
portátil 10 s -> reconectar y TCP 10 s. En paralelo lee el USB para ver que el ESP32 sigue
igual. Siempre termina volviendo a eduroam y comprobando Internet.
Uso: python -I prueba_wifi.py <perfil.xml> <COM> <salida.json>
"""
import json, socket, subprocess, sys, threading, time
import serial

perfil_xml, puerto, salida = sys.argv[1], sys.argv[2], sys.argv[3]
AP, PUERTO_TCP, PERFIL_VUELTA = "192.168.4.1", 3333, "eduroam"
t0 = time.time()
eventos, usb, tcp = [], [], []
invalidas = {"usb": 0, "tcp": 0}


def ev(texto):
    eventos.append([round(time.time() - t0, 1), texto])


def netsh(*args):
    r = subprocess.run(["netsh", "wlan", *args], capture_output=True, text=True, encoding="mbcs", errors="replace")
    ev("netsh " + " ".join(args) + " -> " + r.stdout.strip().splitlines()[-1] if r.stdout.strip() else "netsh " + " ".join(args))
    return r


def ip_kart():
    out = subprocess.run(["ipconfig"], capture_output=True, text=True, encoding="mbcs", errors="replace").stdout
    return "192.168.4." in out


def esperar(cond, segundos):
    fin = time.time() + segundos
    while time.time() < fin:
        if cond():
            return True
        time.sleep(0.5)
    return False


parar_usb = threading.Event()


def lector_usb():
    s = serial.Serial()
    s.port, s.baudrate, s.timeout, s.dtr, s.rts = puerto, 115200, 0.2, False, False
    s.open()
    buf = b""
    while not parar_usb.is_set():
        buf += s.read(512)
        *lineas, buf = buf.split(b"\n")
        for l in lineas:
            if l.startswith(b"{"):
                try:
                    usb.append([round(time.time() - t0, 2), json.loads(l)])
                except ValueError:
                    invalidas["usb"] += 1
    s.close()


def leer_tcp(fase, segundos):
    try:
        c = socket.create_connection((AP, PUERTO_TCP), timeout=5)
    except OSError as e:
        ev(f"{fase}: conexión TCP fallida: {e}")
        return
    ev(f"{fase}: TCP conectado")
    c.settimeout(1)
    buf, fin = b"", time.time() + segundos
    while time.time() < fin:
        try:
            d = c.recv(4096)
        except socket.timeout:
            continue
        except OSError as e:
            ev(f"{fase}: TCP error {e}")
            break
        if not d:
            ev(f"{fase}: TCP cerrado por el ESP32")
            break
        buf += d
        *lineas, buf = buf.split(b"\n")
        for l in lineas:
            if l.startswith(b"{"):
                try:
                    tcp.append([fase, round(time.time() - t0, 2), json.loads(l)])
                except ValueError:
                    invalidas["tcp"] += 1
            elif l.strip():
                ev(f"{fase}: texto TCP: {l.decode(errors='replace')[:160]}")
    c.close()
    ev(f"{fase}: TCP cerrado por el portátil")


hilo = threading.Thread(target=lector_usb, daemon=True)
hilo.start()
try:
    if perfil_xml != "-":
        netsh("add", "profile", f"filename={perfil_xml}", "user=current")
    netsh("connect", "name=kart-esp32")
    ev("IP 192.168.4.x: " + str(esperar(ip_kart, 25)))
    time.sleep(2)
    leer_tcp("A_30s", 30)
    time.sleep(5)
    leer_tcp("B_reconexion", 10)
    netsh("disconnect")
    ev("Wi-Fi del portátil cortado 10 s (el USB sigue leyendo)")
    time.sleep(10)
    netsh("connect", "name=kart-esp32")
    ev("IP de nuevo: " + str(esperar(ip_kart, 25)))
    time.sleep(2)
    leer_tcp("C_tras_corte", 10)
finally:
    netsh("connect", f"name={PERFIL_VUELTA}")
    ok = esperar(lambda: subprocess.run(["ping", "-n", "1", "-w", "1500", "1.1.1.1"], capture_output=True).returncode == 0, 60)
    ev("Internet tras volver a eduroam: " + str(ok))
    parar_usb.set()
    hilo.join(timeout=3)
    json.dump({"eventos": eventos, "usb": usb, "tcp": tcp, "invalidas":invalidas}, open(salida, "w", encoding="utf-8"), ensure_ascii=False)
    print("\n".join(f"{t:7.1f}  {e}" for t, e in eventos))
