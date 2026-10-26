#include "kart_storage.h"
#include "main.h"
#include "bsp_driver_sd.h"
uint8_t Kart_StoragePresent(void){return BSP_SD_IsDetected()==SD_PRESENT;}
uint32_t Kart_StorageUid(unsigned index){const volatile uint32_t* uid=(const volatile uint32_t*)UID_BASE;return index<3?uid[index]:0;}
