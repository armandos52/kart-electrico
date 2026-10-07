// Telemetría de SOLO LECTURA del ESP32 trasero (PROTOCOLO.md v1, una línea JSON por muestra).
//
// El bucle de control solo copia variables y encola la línea sin esperar (telemetria_publicar).
// Wi-Fi (punto de acceso WPA2) y TCP viven en una tarea propia del núcleo 0: un cliente lento,
// la ausencia de clientes o una desconexión descartan tramas, nunca frenan loop().
// No hay ningún camino de entrada: lo que llega por TCP se lee y se descarta.
#pragma once
#include <Arduino.h>

#define TELEMETRIA_PUERTO_TCP   3333
#define TELEMETRIA_PERIODO_MS   200
#define TELEMETRIA_LINEA_MAX    640
#define TELEMETRIA_COLA         4

// Copia de las variables del control en un instante. Los mandos CAN solo son válidos
// tras recibir la primera orden (mandos_validos).
struct MuestraKart {
    uint32_t tick_ms;
    bool mandos_validos;
    uint16_t acelerador_raw;
    uint16_t direcao_raw;
    uint8_t travao;
    uint8_t modo_d;
    uint8_t modo_r;
    uint8_t modo_controlo;
    int motor_esq_speed;
    int motor_dir_speed;
    int16_t rpm_esq;
    int16_t rpm_dir;
    float yaw_rate;
    bool relay1;
    bool relay2;
    uint8_t estado_relays;
    bool carregador;
    bool emergency_stop;
    uint32_t can_rx_age_ms;
    uint32_t can_rx_count;
};

// Arranca la tarea de red (no bloquea: el punto de acceso se crea dentro de la tarea).
void telemetria_iniciar();

// Llamar una vez por vuelta de loop(); mide el periodo del bucle y, cada
// TELEMETRIA_PERIODO_MS, devuelve true para que el bucle tome una muestra.
bool telemetria_toca(uint32_t ahora_ms);

// Formatea y encola sin esperar. También escribe por USB si cabe entero en el búfer de TX.
void telemetria_publicar(const MuestraKart &m);
