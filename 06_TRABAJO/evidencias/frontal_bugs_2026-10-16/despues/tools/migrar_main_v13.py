"""Transformación inicial trazable del v0; ejecutar una sola vez en la copia."""
from pathlib import Path
import re
ROOT=Path(__file__).resolve().parents[1]
p=ROOT/'Core/Src/main.c'
s=p.read_text(encoding='utf-8-sig')
assert 'void Read_Sensors(SensorData_t *data)' in s, 'No es v0: no aplicar de nuevo'
s=s.replace('#include <stdbool.h>','#include <stdbool.h>\n#include "kart_frontend.h"')
def block(name,value):
    global s
    pattern=rf'(/\* USER CODE BEGIN {re.escape(name)} \*/).*?(/\* USER CODE END {re.escape(name)} \*/)'
    s,count=re.subn(pattern,lambda m:m[1]+'\n'+value+'\n'+m[2],s,count=1,flags=re.S)
    assert count==1,name
block('PV','static FMC_SDRAM_CommandTypeDef Command;')
block('PFP','void GetManufacturerId(uint8_t *manufacturer_id);\nvoid EnableMemoryMappedMode(uint8_t manufacturer_id);')
start=s.index('void HAL_CAN_RxFifo0MsgPendingCallback(')
end=s.index('void GetManufacturerId(',start)
s=s[:start]+s[end:]
start=s.index('void CAN_Init_Hardware(void)\n{')
end=s.index('/* USER CODE END 0 */',start)
s=s[:start]+s[end:]
start=s.index('/* USER CODE BEGIN 4 */')
end=s.index(' /* MPU Configuration */',start)
s=s[:start]+'''/* USER CODE BEGIN 4 */
void StartSDTask(void *argument)
{
    (void)argument;
    for (;;) { Kart_SdTick(); osDelay(50); }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/* Control propio en kart_frontend.c; no regenerar desde un .ioc antiguo. */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    /* USER CODE BEGIN 5 */
    (void)argument;
    Kart_Init();
    uint32_t wake = osKernelGetTickCount();
    for (;;) {
        Kart_ControlTick();
        wake += 5;
        uint32_t now = osKernelGetTickCount();
        if ((int32_t)(wake - now) > 0) osDelayUntil(wake);
        else { wake = now; osDelay(1); }
    }
    /* USER CODE END 5 */
}

'''+s[end:]
block('Error_Handler_Debug','Kart_Fatal();')
# Eliminar defines muertos de adquisición/control; mantener SDRAM y framebuffer usados por init.
s=re.sub(r'^#define (?:SD_LOG_INTERVAL_MS|SENSOR_READ_INTERVAL_MS|HALL_CALC_INTERVAL_MS|CAN_ID_ESP32_DATA|CAN_ID_KART_CONTROL|ADC_ACCEL_CHANNEL|ADC_STEER_CHANNEL|BRAKE_PIN|BRAKE_PORT|MODE_D_PIN|MODE_D_PORT|MODE_R_PIN|MODE_R_PORT|CAN_ID_MODO_CONTROLO|MODO_DIFERENCIAL|MODO_DIFERENCIAL_ESC|MODO_AUTOBLOCANTE|ACCEL_NOISE_THRESHOLD|ACCEL_ADC_MIN|ACCEL_ADC_MAX)\s+.*\n','',s,flags=re.M)
s=s.replace('hcan1.Init.AutoRetransmission = ENABLE;','hcan1.Init.AutoRetransmission = DISABLE;')
# SD task definitions/prototype quedan dentro de zonas USER CODE para regeneración.
a=s.index('/* Definitions for SDTask */');b=s.index('/* USER CODE BEGIN PV */',a)
sd=s[a:b];s=s[:a]+s[b:];s=s.replace('/* USER CODE BEGIN PV */','/* USER CODE BEGIN PV */\n'+sd,1)
s=s.replace('void StartSDTask(void *argument);\n','')
s=s.replace('/* USER CODE BEGIN PFP */','/* USER CODE BEGIN PFP */\nvoid StartSDTask(void *argument);',1)
s=s.replace('  /* USER CODE END RTOS_THREADS */','  if (!defaultTaskHandle || !TouchGFXTaskHandle || !videoTaskHandle || !SDTaskHandle) Kart_Fatal();\n  /* USER CODE END RTOS_THREADS */')
p.write_text(s,encoding='utf-8')
conf=ROOT/'Core/Inc/stm32f7xx_hal_conf.h'
text=conf.read_text(encoding='utf-8-sig').replace('/* #define HAL_IWDG_MODULE_ENABLED */','#define HAL_IWDG_MODULE_ENABLED')
conf.write_text(text,encoding='utf-8')
irq=ROOT/'Core/Src/stm32f7xx_it.c'
text=irq.read_text(encoding='utf-8-sig')
assert 'void EXTI4_IRQHandler(' not in text
text=text.replace('/* USER CODE BEGIN 1 */','''/* USER CODE BEGIN 1 */
void EXTI4_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(HALL_DI_ESQ_Pin); }
void EXTI9_5_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(HALL_DI_DIR_Pin); }
''',1)
irq.write_text(text,encoding='utf-8')
print('main.c extraído a kart_frontend.c, hooks USER CODE, IWDG y EXTI configurados en copia.')
