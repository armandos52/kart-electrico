#ifndef KART_STORAGE_H
#define KART_STORAGE_H
#include <stdint.h>
/* Board-specific SD adapter. Only the low priority storage task calls it. */
uint8_t Kart_StoragePresent(void);
uint32_t Kart_StorageUid(unsigned index);
#endif
