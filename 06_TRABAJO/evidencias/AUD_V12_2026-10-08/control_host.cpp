#include "stubs.hpp"
#line 1 "ESP32-final.ino"






#define CAN_CS_PIN 5
#define SPI_MOSI   23
#define SPI_MISO   19
#define SPI_SCK    18
#define MPU_SDA     21
#define MPU_SCL     22

#define HALL_TRAS_ESQ       34
#define MOTOR_PWM_ESQ       32
#define TRAVAO_PWM_ESQ      33
#define SW_DIR_F_ESQ        25
#define SW_DIR_MT_ESQ       26
#define SW_TRAVAO_ESQ       27

#define HALL_TRAS_DIR       4
#define MOTOR_PWM_DIR       2
#define TRAVAO_PWM_DIR      15
#define SW_DIR_F_DIR        13
#define SW_DIR_MT_DIR       12
#define SW_TRAVAO_DIR       14

#define RELAY1_PIN          16
#define RELAY2_PIN          17
#define PIN_CARREGADOR      35

#define PWM_FREQ          5000
#define PWM_RESOLUTION    8
#define PWM_MAX           255
#define ADC_MAX           4095
#define ADC_CENTER        2047

#define REVERSE_SPEED_LIMIT 1.0
#define PULSOS_POR_VOLTA_ESQ   89
#define PULSOS_POR_VOLTA_DIR   68
#define HALL_CALC_INTERVAL 500
#define MIN_PULSOS_VALIDOS 2
#define LEFT_MOTOR_INVERT true

#define RAMP_RPM_PER_SEC    300.0

#define L_DISTANCIA_EIXOS   1.3
#define W_BITOLA            1.0
#define V_MAX_MOTOR         570.0
#define RAIO_RODA           0.243

unsigned long last_command_time = 0;
const unsigned long COMMAND_TIMEOUT = 100;

uint16_t acelerador_raw = 0;
uint16_t direcao_raw = 0;
uint8_t travao = 0;
uint8_t modo_d = 0;
uint8_t modo_r = 0;
uint8_t modo_controlo = 0;
uint8_t modo_anterior = 2;

int motor_esq_speed = 255;
int motor_dir_speed = 255;

bool command_received = false;
bool emergency_stop = false;
uint32_t can_rx_count = 0;  // v1: órdenes 0x100 válidas recibidas (diagnóstico)
bool mpu_ok = false;        // v1.1: resultado de testConnection(), para no publicar guiñada sin MPU
bool control_ready = false;
bool can_init_ok = false;
uint32_t can_init_attempts = 0;
bool start_requested = false;
const char *control_state = "can_wait";
const char *drive_inhibit = "can_init";

volatile uint32_t pulsos_esq = 0;
volatile uint32_t pulsos_dir = 0;
int16_t rpm_esq = 0;
int16_t rpm_dir = 0;
int16_t rpm_medio = 0;
int16_t velocidade_kph = 0;
uint32_t last_hall_calc = 0;

MPU6050 mpu;
float yaw_rate = 0.0;
float gyro_offset_z = 0.0;

MCP_CAN CAN(CAN_CS_PIN);

enum EstadoRelays { 
    DESLIGADO,       
    LIGOU_RELE1,     
    LIGOU_RELE2,     
    PRONTO           
};
EstadoRelays estado_relays = DESLIGADO;
unsigned long tempo_relay = 0;
bool start_anterior = false;

static float pwm_atual = 255.0;
static unsigned long last_ramp_time = 0;

void processarComando();
void pararMotores();
void pararMotor();
void ativarDirecao(bool frente, bool re);
void atualizarRPM();
void enviarRPM();
void processarSequenciaRelays(bool start_pressed);
void tomarMuestra(unsigned long current_time);
void IRAM_ATTR hall_esq_isr();
void IRAM_ATTR hall_dir_isr();

float calcular_angulo(uint16_t adc);
void modo_dif(int acelerador_pct);
void modo_esc(int acelerador_pct);
void modo_blq(int acelerador_pct);

void IRAM_ATTR hall_esq_isr() { pulsos_esq++; }
void IRAM_ATTR hall_dir_isr() { pulsos_dir++; }

float calcular_angulo(uint16_t adc) {
    float angulo = (adc - 2047.0) / 47.9;
    if (angulo > 20.0) angulo = 20.0;
    if (angulo < -20.0) angulo = -20.0;
    return angulo;
}

void modo_dif(int acelerador_pct) {
    int pwm_base = map(acelerador_pct, 0, 100, PWM_MAX, 0);
    pwm_base = constrain(pwm_base, 0, PWM_MAX);
    int offset_adc = (int)direcao_raw - ADC_CENTER;
    int pwm_esq = pwm_base;
    int pwm_dir = pwm_base;
    
    if (abs(offset_adc) > 200) {
        float V = ((float)(PWM_MAX - pwm_base) / (float)PWM_MAX) * 570.0;
        float angulo = abs(calcular_angulo(direcao_raw));
        if (angulo > 25.0) angulo = 25.0;
        
        float Rmeio = 1.3 / tan(angulo * PI / 180.0);
        float Rint = Rmeio - 0.5;
        float Rext = Rmeio + 0.5;
        
        float Vint = V * (Rint / Rmeio);
        float Vext = V * (Rext / Rmeio);
        
        if (Vint > 570.0) Vint = 570.0;
        if (Vext > 570.0) Vext = 570.0;
        if (Vint < 0.0) Vint = 0.0;
        if (Vext < 0.0) Vext = 0.0;
        
        int pwm_int = (int)(PWM_MAX - (Vint / 570.0) * PWM_MAX);
        int pwm_ext = (int)(PWM_MAX - (Vext / 570.0) * PWM_MAX);
        
        if (offset_adc > 200) {
            pwm_esq = pwm_ext; pwm_dir = pwm_int;
        } else {
            pwm_esq = pwm_int; pwm_dir = pwm_ext;
        }
    }
    
    pwm_esq = constrain(pwm_esq, 0, PWM_MAX);
    pwm_dir = constrain(pwm_dir, 0, PWM_MAX);
    motor_esq_speed = pwm_esq;
    motor_dir_speed = pwm_dir;
    ledcWrite(MOTOR_PWM_ESQ, pwm_esq);
    ledcWrite(MOTOR_PWM_DIR, pwm_dir);
}

void modo_esc(int acelerador_pct) {
    int pwm_base = map(acelerador_pct, 0, 100, PWM_MAX, 0);
    pwm_base = constrain(pwm_base, 0, PWM_MAX);
    int offset_adc = (int)direcao_raw - ADC_CENTER;
    int pwm_esq = pwm_base;
    int pwm_dir = pwm_base;
    
    if (abs(offset_adc) > 200) {
        float V = ((float)(PWM_MAX - pwm_base) / (float)PWM_MAX) * 570.0;
        float angulo = abs(calcular_angulo(direcao_raw));
        if (angulo > 25.0) angulo = 25.0;
        
        float Rmeio = 1.3 / tan(angulo * PI / 180.0);
        float Rint = Rmeio - 0.5;
        float Rext = Rmeio + 0.5;
        
        float Vint = V * (Rint / Rmeio);
        float Vext = V * (Rext / Rmeio);
        
        float V_ms = V * (2.0 * PI * 0.243) / 60.0;
        float YAW_IDEAL = 0.0;
        if (angulo > 0.5) {
            YAW_IDEAL = ((V_ms * tan(angulo * PI / 180.0)) / 1.3) * 57.2958;
        }
        
        float dif_yaw = YAW_IDEAL - yaw_rate;
        
        if (abs(dif_yaw) > 2.0) {
            if (dif_yaw > 0) {
                Vext = Vint * (Rext/Rint + abs(dif_yaw)/200.0);
            } else {
                Vext = V * (Rext / Rmeio);
                Vint = Vext / (Rext/Rint + abs(dif_yaw)/200.0);
            }
        }
        
        if (Vint > 570.0) Vint = 570.0;
        if (Vext > 570.0) Vext = 570.0;
        if (Vint < 0.0) Vint = 0.0;
        if (Vext < 0.0) Vext = 0.0;
        
        int pwm_int = (int)(PWM_MAX - (Vint / 570.0) * PWM_MAX);
        int pwm_ext = (int)(PWM_MAX - (Vext / 570.0) * PWM_MAX);
        
        if (offset_adc > 200) {
            pwm_esq = pwm_ext; pwm_dir = pwm_int;
        } else {
            pwm_esq = pwm_int; pwm_dir = pwm_ext;
        }
    }
    
    pwm_esq = constrain(pwm_esq, 0, PWM_MAX);
    pwm_dir = constrain(pwm_dir, 0, PWM_MAX);
    motor_esq_speed = pwm_esq;
    motor_dir_speed = pwm_dir;
    ledcWrite(MOTOR_PWM_ESQ, pwm_esq);
    ledcWrite(MOTOR_PWM_DIR, pwm_dir);
}

void modo_blq(int acelerador_pct) {
    int pwm = map(acelerador_pct, 0, 100, PWM_MAX, 0);
    pwm = constrain(pwm, 0, PWM_MAX);
    motor_esq_speed = pwm;
    motor_dir_speed = pwm;
    ledcWrite(MOTOR_PWM_ESQ, pwm);
    ledcWrite(MOTOR_PWM_DIR, pwm);
}

void setup() {
  pinMode(RELAY1_PIN, OUTPUT); 
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(PIN_CARREGADOR, INPUT);
  digitalWrite(RELAY1_PIN, LOW); 
  digitalWrite(RELAY2_PIN, LOW);

  Serial.setTxBufferSize(2048);  // cabe la muestra completa incluso con mandos y diagnóstico
  Serial.begin(115200);
  telemetria_iniciar();  // v1.2: visible también sin MCP2515; no activa el control
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CAN_CS_PIN);

  while (!can_init_ok) {
    can_init_attempts++;
    can_init_ok = CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK;
    if (can_init_ok) can_init_ok = CAN.setMode(MCP_NORMAL) == CAN_OK;
    if (!can_init_ok) {
      Serial.println("CAN init failed, retrying...");
      unsigned long espera = millis();
      do {
        if (telemetria_toca(millis())) tomarMuestra(millis());
        delay(10);
      } while (millis() - espera < 500);
    }
  }
  Serial.println("CAN OK!");

  control_state = "mpu_init";
  drive_inhibit = "initializing";
  tomarMuestra(millis());
  Wire.begin(MPU_SDA, MPU_SCL);
  mpu.initialize();
  mpu_ok = mpu.testConnection();  // v1.1: misma llamada, resultado guardado
  Serial.println(mpu_ok ? "MPU-6050 OK!" : "MPU-6050 FALHOU!");

  Serial.println("A calibrar giroscopio...");
  float soma_z = 0;
  for (int i = 0; mpu_ok && i < 500; i++) {
    int16_t gx = 0, gy = 0, gz = 0;
    mpu.getRotation(&gx, &gy, &gz);
    soma_z += gz;
    delay(2);
  }
  gyro_offset_z = mpu_ok ? soma_z / 500.0 : 0.0;

  pinMode(MOTOR_PWM_ESQ, OUTPUT);
  pinMode(SW_DIR_F_ESQ, OUTPUT); 
  pinMode(SW_DIR_MT_ESQ, OUTPUT); 
  pinMode(SW_TRAVAO_ESQ, OUTPUT);
  pinMode(TRAVAO_PWM_ESQ, OUTPUT);
  digitalWrite(SW_DIR_F_ESQ, LOW); 
  digitalWrite(SW_DIR_MT_ESQ, LOW); 
  digitalWrite(SW_TRAVAO_ESQ, LOW);

  pinMode(MOTOR_PWM_DIR, OUTPUT);
  pinMode(SW_DIR_F_DIR, OUTPUT); 
  pinMode(SW_DIR_MT_DIR, OUTPUT); 
  pinMode(SW_TRAVAO_DIR, OUTPUT);
  pinMode(TRAVAO_PWM_DIR, OUTPUT);
  digitalWrite(SW_DIR_F_DIR, LOW); 
  digitalWrite(SW_DIR_MT_DIR, LOW); 
  digitalWrite(SW_TRAVAO_DIR, LOW);

  ledcAttach(MOTOR_PWM_ESQ, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(MOTOR_PWM_DIR, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(TRAVAO_PWM_ESQ, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(TRAVAO_PWM_DIR, PWM_FREQ, PWM_RESOLUTION);

  ledcWrite(MOTOR_PWM_ESQ, 255); 
  ledcWrite(MOTOR_PWM_DIR, 255);
  ledcWrite(TRAVAO_PWM_ESQ, 0); 
  ledcWrite(TRAVAO_PWM_DIR, 0);

  pinMode(HALL_TRAS_ESQ, INPUT); 
  pinMode(HALL_TRAS_DIR, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL_TRAS_ESQ), hall_esq_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(HALL_TRAS_DIR), hall_dir_isr, FALLING);

  pararMotores();
  last_ramp_time = millis();
  Serial.println("KART ELÉTRICO - DIF | ESC | BLQ");
  control_ready = true;
  control_state = "running";
  drive_inhibit = "no_command";
  telemetria_control_iniciado();
}

void loop() {
  delay(1);
  unsigned long current_time = millis();

  static unsigned long tempo_verificacao = 0;
  static bool timer_ativo = false;
  
  bool reles_on = (digitalRead(RELAY1_PIN) == HIGH || digitalRead(RELAY2_PIN) == HIGH);
  
  if (reles_on) {
      if (!timer_ativo) {
          timer_ativo = true;
          tempo_verificacao = current_time;
      }
      if (current_time - tempo_verificacao >= 2000) {
          if (digitalRead(PIN_CARREGADOR) == HIGH) {
              digitalWrite(RELAY1_PIN, LOW);
              digitalWrite(RELAY2_PIN, LOW);
              estado_relays = DESLIGADO;
              Serial.println("Carregador inserido! Sistema desligado.");
          }
          timer_ativo = false;
      }
  } else {
      timer_ativo = false;
  }

  switch (estado_relays) {
      case LIGOU_RELE1:
          if (current_time - tempo_relay >= 2000) {
              digitalWrite(RELAY2_PIN, HIGH);
              estado_relays = LIGOU_RELE2;
              tempo_relay = current_time;
              Serial.println("Relé 2 ligado.");
          }
          break;
      
      case LIGOU_RELE2:
          if (current_time - tempo_relay >= 1000) {
              digitalWrite(RELAY1_PIN, LOW);
              estado_relays = PRONTO;
              Serial.println("Relé 1 desligado. Sistema pronto.");
          }
          break;
      
      case PRONTO:
          break;
      
      default:
          break;
  }

  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    unsigned long id; byte len = 0; byte buf[8];
    byte lectura_ok = CAN.readMsgBuf(&id, &len, buf);

    if (lectura_ok == CAN_OK && id == 0x100 && len == 8) {
      acelerador_raw = (buf[0] << 8) | buf[1];
      direcao_raw    = (buf[2] << 8) | buf[3];
      travao         = buf[4];
      modo_d         = buf[5];
      modo_r         = buf[6];
      modo_controlo  = buf[7] & 0x03;
      uint8_t start_stop = (buf[7] >> 2) & 0x01;
      start_requested = start_stop == 1;

      processarSequenciaRelays(start_stop == 1);
      command_received = true; emergency_stop = false;
      can_rx_count++;  // v1
      last_command_time = current_time;
      processarComando();
    }
  }

  if (command_received && (current_time - last_command_time > COMMAND_TIMEOUT)) {
    pararMotores(); emergency_stop = true; drive_inhibit = "can_timeout";
  }

  atualizarRPM();

  if (mpu_ok) {
    int16_t gx = 0, gy = 0, gz = 0;
    mpu.getRotation(&gx, &gy, &gz);
    yaw_rate = (gz - gyro_offset_z) / 131.0;
  }

  static unsigned long last_tx_time = 0;
  if (current_time - last_tx_time >= 200) {
    last_tx_time = current_time;
    enviarRPM();
  }

  if (telemetria_toca(current_time)) tomarMuestra(current_time);  // v1: solo lectura
}

// v1: copia de variables para la telemetría; no escribe ninguna variable del control.
void tomarMuestra(unsigned long current_time) {
  MuestraKart m{};
  m.tick_ms = current_time;
  m.mandos_validos = command_received;
  m.acelerador_raw = acelerador_raw;
  m.direcao_raw = direcao_raw;
  m.travao = travao;
  m.modo_d = modo_d;
  m.modo_r = modo_r;
  m.modo_controlo = modo_controlo;
  m.motor_esq_speed = motor_esq_speed;
  m.motor_dir_speed = motor_dir_speed;
  m.rpm_esq = rpm_esq;
  m.rpm_dir = rpm_dir;
  m.yaw_rate = yaw_rate;
  m.relay1 = digitalRead(RELAY1_PIN) == HIGH;  // estado ordenado (salida), no medido
  m.relay2 = digitalRead(RELAY2_PIN) == HIGH;
  m.estado_relays = (uint8_t)estado_relays;
  m.carregador = digitalRead(PIN_CARREGADOR) == HIGH;  // nivel del pin, como lo interpreta loop()
  m.emergency_stop = emergency_stop;
  m.can_rx_age_ms = command_received ? (uint32_t)(current_time - last_command_time) : 0;
  m.can_rx_count = can_rx_count;
  m.mpu_ok = mpu_ok;
  m.control_ready = control_ready;
  m.can_init_ok = can_init_ok;
  m.can_init_attempts = can_init_attempts;
  m.start_requested = start_requested;
  m.control_state = control_state;
  m.drive_inhibit = drive_inhibit;
  m.drive_direction = control_ready && digitalRead(SW_DIR_F_DIR) == HIGH ? "D" :
                      (control_ready && digitalRead(SW_DIR_MT_DIR) == HIGH ? "R" : "N");
  telemetria_publicar(m);
}

void processarSequenciaRelays(bool start_pressed) {
  if (start_pressed && !start_anterior) {
    if (estado_relays == DESLIGADO) {
      digitalWrite(RELAY1_PIN, HIGH);
      estado_relays = LIGOU_RELE1;
      tempo_relay = millis();
      Serial.println("Relé 1 ligado.");
    }
  }
  else if (!start_pressed && start_anterior) {
    if (estado_relays != DESLIGADO && (millis() - tempo_relay) > 500) {
      digitalWrite(RELAY1_PIN, LOW);
      digitalWrite(RELAY2_PIN, LOW);
      estado_relays = DESLIGADO;
      Serial.println("Sistema desligado (STOP)");
    }
  }
  start_anterior = start_pressed;
}

void atualizarRPM() {
  unsigned long current_time = millis();
  if (current_time - last_hall_calc >= HALL_CALC_INTERVAL) {
    uint32_t p_esq, p_dir;
    noInterrupts(); p_esq = pulsos_esq; p_dir = pulsos_dir;
    pulsos_esq = 0; pulsos_dir = 0; interrupts();
    last_hall_calc = current_time;
    
    const float fator_rpm_esq = 60.0f / (PULSOS_POR_VOLTA_ESQ * (HALL_CALC_INTERVAL / 1000.0f));
    const float fator_rpm_dir = 60.0f / (PULSOS_POR_VOLTA_DIR * (HALL_CALC_INTERVAL / 1000.0f));
    
    rpm_esq = (p_esq >= MIN_PULSOS_VALIDOS) ? (int16_t)(p_esq * fator_rpm_esq) : 0;
    rpm_dir = (p_dir >= MIN_PULSOS_VALIDOS) ? (int16_t)(p_dir * fator_rpm_dir) : 0;
    rpm_medio = (rpm_esq + rpm_dir) / 2;
    velocidade_kph = (int16_t)(rpm_medio * 3.14159265 * 0.48 * 3.6 / 60.0);
  }
}

void enviarRPM() {
  byte data[8];
  data[0] = (velocidade_kph >> 8) & 0xFF; data[1] = velocidade_kph & 0xFF;
  data[2] = (rpm_esq >> 8) & 0xFF; data[3] = rpm_esq & 0xFF;
  data[4] = (rpm_dir >> 8) & 0xFF; data[5] = rpm_dir & 0xFF;
  int16_t yaw_int = (int16_t)(yaw_rate * 10);
  data[6] = (yaw_int >> 8) & 0xFF;
  data[7] = yaw_int & 0xFF;
  CAN.sendMsgBuf(0x123, 0, 8, data);
}

void ativarDirecao(bool frente, bool re) {
  digitalWrite(SW_DIR_F_DIR, frente ? HIGH : LOW);
  digitalWrite(SW_DIR_MT_DIR, re ? HIGH : LOW);
  if (LEFT_MOTOR_INVERT) {
    digitalWrite(SW_DIR_F_ESQ, re ? HIGH : LOW);
    digitalWrite(SW_DIR_MT_ESQ, frente ? HIGH : LOW);
  } else {
    digitalWrite(SW_DIR_F_ESQ, frente ? HIGH : LOW);
    digitalWrite(SW_DIR_MT_ESQ, re ? HIGH : LOW);
  }
}

void pararMotor() {
  ledcWrite(MOTOR_PWM_ESQ, 255); ledcWrite(MOTOR_PWM_DIR, 255);
  motor_esq_speed = 255; motor_dir_speed = 255;
  pwm_atual = 255.0;
}

void processarComando() {
  if (estado_relays != LIGOU_RELE2 && estado_relays != PRONTO) { drive_inhibit = "relays"; pararMotores(); return; }
  bool rpm_zero = (rpm_medio == 0);
  if (travao == 1) { drive_inhibit = "brake"; pararMotores(); return; }

  int acelerador_pct = map(acelerador_raw, 0, ADC_MAX, 0, 100);
  acelerador_pct = constrain(acelerador_pct, 0, 100);

  if (acelerador_pct <= 0) { drive_inhibit = "pedal_zero"; pararMotor(); return; }
  if (modo_controlo == 1 && !mpu_ok) { drive_inhibit = "mpu_unavailable"; pararMotores(); return; }
  drive_inhibit = "none";

  float pwm_alvo = map(acelerador_pct, 0, 100, PWM_MAX, 0);
  
  unsigned long now = millis();
  float dt = (now - last_ramp_time) / 1000.0;
  if (dt < 0.001) dt = 0.001;
  if (dt > 0.1) dt = 0.1;
  last_ramp_time = now;
  
  float ramp_pwm_per_sec = (RAMP_RPM_PER_SEC / 570.0) * 255.0;
  
  if (pwm_alvo < pwm_atual) {
    pwm_atual -= ramp_pwm_per_sec * dt;
    if (pwm_atual < pwm_alvo) pwm_atual = pwm_alvo;
  } else {
    pwm_atual = pwm_alvo;
  }
  
  int pwm_base = (int)pwm_atual;
  pwm_base = constrain(pwm_base, 0, PWM_MAX);

  if (modo_controlo == 0) {
    modo_dif(acelerador_pct);
  }
  else if (modo_controlo == 1) {
    modo_esc(acelerador_pct);
  }
  else {
    modo_blq(acelerador_pct);
  }

  if (modo_d == 1) {
    if (rpm_zero || modo_anterior == 0) {
      modo_anterior = 0; ativarDirecao(true, false);
    } else { drive_inhibit = "direction_change"; pararMotores(); }
  }
  else if (modo_r == 1) {
    if (rpm_zero || modo_anterior == 1) {
      modo_anterior = 1; ativarDirecao(false, true);
    } else { drive_inhibit = "direction_change"; pararMotores(); }
  }
  else { drive_inhibit = "neutral"; modo_anterior = 2; pararMotores(); }
}

void pararMotores() {
  digitalWrite(SW_DIR_F_ESQ, LOW); digitalWrite(SW_DIR_MT_ESQ, LOW);
  digitalWrite(SW_TRAVAO_ESQ, LOW);
  ledcWrite(MOTOR_PWM_ESQ, 255); ledcWrite(TRAVAO_PWM_ESQ, 0);
  digitalWrite(SW_DIR_F_DIR, LOW); digitalWrite(SW_DIR_MT_DIR, LOW);
  digitalWrite(SW_TRAVAO_DIR, LOW);
  ledcWrite(MOTOR_PWM_DIR, 255); ledcWrite(TRAVAO_PWM_DIR, 0);
  motor_esq_speed = 255; motor_dir_speed = 255;
  pwm_atual = 255.0;
}

#include "casos.cpp"
