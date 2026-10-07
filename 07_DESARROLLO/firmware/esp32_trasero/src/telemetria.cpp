// Telemetría de solo lectura: ver include/telemetria.h y 07_DESARROLLO/panel_local/PROTOCOLO.md.
#include "telemetria.h"

#include <WiFi.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdarg.h>

#if __has_include("secretos.h")
#include "secretos.h"
#else
#error "Falta include/secretos.h: copia include/secretos.example.h y pon una contraseña WPA2 propia"
#endif

#ifndef FW_VERSION
#define FW_VERSION "esp32_trasero-sin-version"
#endif

namespace {

struct Linea {
    uint16_t len;
    char texto[TELEMETRIA_LINEA_MAX];
};

const IPAddress IP_AP(192, 168, 4, 1);
const IPAddress MASCARA(255, 255, 255, 0);
const int MAX_CLIENTES = 2;

QueueHandle_t cola = nullptr;
char sesion[17] = "sin-iniciar";
uint32_t seq = 0;

// Contadores: cada uno lo escribe una sola tarea (lecturas de 32 bits atómicas en ESP32).
uint32_t descartadas_cola = 0;          // loop(): cola llena
volatile uint32_t descartadas_red = 0;  // tarea de red: escritura incompleta a un cliente
volatile uint8_t clientes_activos = 0;

uint32_t ultimo_us = 0;
uint32_t periodo_max_us = 0;   // periodo máximo de loop() desde la última muestra
uint32_t publicar_max_us = 0;  // coste máximo de telemetria_publicar() en loop()
uint32_t ultimo_envio_ms = 0;

const char *motivo_reset() {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON:   return "encendido";
        case ESP_RST_EXT:       return "pin_externo";
        case ESP_RST_SW:        return "software";
        case ESP_RST_PANIC:     return "excepcion_panic";
        case ESP_RST_INT_WDT:   return "watchdog_interrupcion";
        case ESP_RST_TASK_WDT:  return "watchdog_tarea";
        case ESP_RST_WDT:       return "watchdog_otro";
        case ESP_RST_DEEPSLEEP: return "deep_sleep";
        case ESP_RST_BROWNOUT:  return "brownout_tension";
        case ESP_RST_SDIO:      return "sdio";
        default:                return "desconocido";
    }
}

bool agregar(Linea &l, const char *formato, ...) {
    if (l.len >= sizeof(l.texto)) return false;
    va_list args;
    va_start(args, formato);
    int n = vsnprintf(l.texto + l.len, sizeof(l.texto) - l.len, formato, args);
    va_end(args);
    if (n < 0 || l.len + n >= sizeof(l.texto)) {
        l.len = sizeof(l.texto);  // marca desbordamiento
        return false;
    }
    l.len += n;
    return true;
}

void enviar_cabecera(NetworkClient &c) {
    char texto[200];
    int n = snprintf(texto, sizeof(texto),
                     "# kart esp32 fw=%s session=%s reset=%s lectura_solo=1 periodo_ms=%d\n",
                     FW_VERSION, sesion, motivo_reset(), TELEMETRIA_PERIODO_MS);
    if (n > 0) c.write(reinterpret_cast<const uint8_t *>(texto), min(n, (int)sizeof(texto) - 1));
}

void tarea_red(void *) {
    // No guardar la configuración del punto de acceso en NVS: la flash no se escribe.
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IP_AP, IP_AP, MASCARA);
    bool ap_ok = WiFi.softAP(KART_WIFI_SSID, KART_WIFI_PASSWORD, 6, 0, MAX_CLIENTES);
    // Potencia reducida: alcance de banco y menos picos de corriente (riesgo de brownout).
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    NetworkServer servidor(TELEMETRIA_PUERTO_TCP);
    servidor.begin();
    servidor.setNoDelay(true);
    NetworkClient clientes[MAX_CLIENTES];
    log_i("AP %s: %s, TCP %u", KART_WIFI_SSID, ap_ok ? "activo" : "FALLO", TELEMETRIA_PUERTO_TCP);

    Linea l;
    for (;;) {
        if (servidor.hasClient()) {
            NetworkClient nuevo = servidor.accept();
            int hueco = -1;
            for (int i = 0; i < MAX_CLIENTES; i++)
                if (!clientes[i].connected()) { hueco = i; break; }
            if (hueco < 0) {
                nuevo.stop();  // sin hueco: se rechaza el nuevo, se conserva el existente
            } else {
                nuevo.setNoDelay(true);
                clientes[hueco] = nuevo;
                enviar_cabecera(clientes[hueco]);
            }
        }

        bool hay_linea = xQueueReceive(cola, &l, pdMS_TO_TICKS(50)) == pdTRUE;
        uint8_t activos = 0;
        for (int i = 0; i < MAX_CLIENTES; i++) {
            NetworkClient &c = clientes[i];
            if (!c.connected()) continue;
            while (c.available() > 0) c.read();  // sin entrada: se descarta todo lo recibido
            if (hay_linea) {
                size_t escrito = c.write(reinterpret_cast<const uint8_t *>(l.texto), l.len);
                if (escrito != l.len) {
                    descartadas_red = descartadas_red + 1;
                    c.stop();  // cliente lento o caído: se cierra, el kart no espera
                    continue;
                }
            }
            activos++;
        }
        clientes_activos = activos;
    }
}

}  // namespace

void telemetria_iniciar() {
    snprintf(sesion, sizeof(sesion), "%08lx%08lx", (unsigned long)esp_random(), (unsigned long)esp_random());
    Serial.printf("FW %s | sesion %s | reset %s\n", FW_VERSION, sesion, motivo_reset());
    cola = xQueueCreate(TELEMETRIA_COLA, sizeof(Linea));
    if (cola == nullptr) {
        Serial.println("Telemetria: sin memoria para la cola; desactivada");
        return;
    }
    // Núcleo 0 (el de la pila Wi-Fi), prioridad baja; loop() corre en el núcleo 1.
    xTaskCreatePinnedToCore(tarea_red, "telemetria", 6144, nullptr, 1, nullptr, 0);
    Serial.printf("Telemetria: AP \"%s\" -> TCP %s:%d (solo lectura)\n",
                  KART_WIFI_SSID, IP_AP.toString().c_str(), TELEMETRIA_PUERTO_TCP);
}

bool telemetria_toca(uint32_t ahora_ms) {
    uint32_t ahora_us = micros();
    if (ultimo_us != 0) {
        uint32_t periodo = ahora_us - ultimo_us;
        if (periodo > periodo_max_us) periodo_max_us = periodo;
    }
    ultimo_us = ahora_us;
    if (cola == nullptr || ahora_ms - ultimo_envio_ms < TELEMETRIA_PERIODO_MS) return false;
    ultimo_envio_ms = ahora_ms;
    return true;
}

void telemetria_publicar(const MuestraKart &m) {
    uint32_t t0 = micros();
    Linea l;
    l.len = 0;
    seq++;
    agregar(l, "{\"v\":1,\"type\":\"telemetry\",\"node\":\"esp32\",\"session\":\"%s\",\"seq\":%lu,"
               "\"tick_ms\":%lu,\"fw\":\"%s\",\"signals\":{",
            sesion, (unsigned long)seq, (unsigned long)m.tick_ms, FW_VERSION);

    // Mandos recibidos por CAN (0x100): solo tras la primera orden válida.
    if (m.mandos_validos) {
        int acelerador_pct = constrain(map(m.acelerador_raw, 0, 4095, 0, 100), 0, 100);  // misma fórmula que processarComando()
        const char *marcha = m.modo_d == 1 ? "D" : (m.modo_r == 1 ? "R" : "N");  // misma prioridad que processarComando()
        const char *modo = m.modo_controlo == 0 ? "DIF" : (m.modo_controlo == 1 ? "ESC" : "BLQ");
        agregar(l, "\"accelerator_raw\":%u,\"accelerator_pct\":%d,\"steer_raw\":%u,\"brake\":%d,"
                   "\"gear\":\"%s\",\"mode\":\"%s\",\"can_rx_age_ms\":%lu,",
                m.acelerador_raw, acelerador_pct, m.direcao_raw, m.travao == 1 ? 1 : 0,
                marcha, modo, (unsigned long)m.can_rx_age_ms);
    }
    if (isfinite(m.yaw_rate)) agregar(l, "\"yaw_dps\":%.1f,", m.yaw_rate);
    agregar(l, "\"pwm_left_raw\":%d,\"pwm_right_raw\":%d,\"rpm_rl\":%d,\"rpm_rr\":%d,"
               "\"relay1\":%d,\"relay2\":%d,\"relay_seq\":%u,\"charger_connected\":%d,"
               "\"emergency_stop\":%d,\"can_rx_count\":%lu,\"loop_max_us\":%lu,"
               "\"telemetry_us_max\":%lu,\"telemetry_dropped\":%lu,\"wifi_clients\":%u}}\n",
            m.motor_esq_speed, m.motor_dir_speed, m.rpm_esq, m.rpm_dir,
            m.relay1 ? 1 : 0, m.relay2 ? 1 : 0, m.estado_relays, m.carregador ? 1 : 0,
            m.emergency_stop ? 1 : 0, (unsigned long)m.can_rx_count, (unsigned long)periodo_max_us,
            (unsigned long)publicar_max_us, (unsigned long)(descartadas_cola + descartadas_red),
            clientes_activos);

    periodo_max_us = 0;
    publicar_max_us = 0;
    if (l.len >= sizeof(l.texto)) {  // línea truncada: no se envía JSON incompleto
        descartadas_cola++;
        return;
    }
    // USB: solo si cabe entera en el búfer de TX (no bloquea).
    if (Serial.availableForWrite() >= l.len) Serial.write(l.texto, l.len);
    if (xQueueSend(cola, &l, 0) != pdTRUE) descartadas_cola++;
    uint32_t coste = micros() - t0;
    if (coste > publicar_max_us) publicar_max_us = coste;
}
