"""Falla si una regeneración CubeMX pierde requisitos no expresables solo en .ioc."""
from pathlib import Path
import re

def verify(root):
    main=(root/'Core/Src/main.c').read_text(encoding='utf-8-sig')
    task=main[main.index('const osThreadAttr_t defaultTask_attributes'):main.index('/* Definitions for TouchGFXTask */')]
    required=['.stack_size = 4096,','osPriorityAboveNormal']
    for token in required:
        if token not in task:raise ValueError('Regeneración perdió control: '+token)
    for token in ['hcan1.Init.AutoRetransmission = DISABLE','hcan1.Init.AutoBusOff = ENABLE','Kart_Init();','Kart_ControlTick();','Kart_SdTick();','Kart_Fatal();','!SDTaskHandle']:
        if token not in main:raise ValueError('Regeneración perdió: '+token)
    conf=(root/'Core/Inc/stm32f7xx_hal_conf.h').read_text()
    if not re.search(r'^#define HAL_IWDG_MODULE_ENABLED',conf,re.M):raise ValueError('Falta módulo IWDG')
    irq=(root/'Core/Src/stm32f7xx_it.c').read_text()
    for token in ['void EXTI4_IRQHandler','void EXTI9_5_IRQHandler']:
        if irq.count(token)!=1:raise ValueError('IRQ ausente o duplicada: '+token)
    rt=(root/'Core/Inc/FreeRTOSConfig.h').read_text()
    for token in ['configCHECK_FOR_STACK_OVERFLOW 2','configUSE_MALLOC_FAILED_HOOK 1']:
        if token not in rt:raise ValueError('Falta supervisión RTOS: '+token)
    print('Configuración comprobada: control, CAN, IWDG, IRQ y RTOS.')

if __name__=='__main__':verify(Path(__file__).resolve().parents[1])
