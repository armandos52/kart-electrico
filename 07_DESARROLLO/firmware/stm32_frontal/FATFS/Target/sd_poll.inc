/* ENS-0029: SDTask waits cooperatively for DMA; polling TX FIFO underrun was
 * observed on the real board (HAL error 0x10). Dedicated cache-line-aligned
 * bounce buffer, one operation at a time, no queued stale completions. */
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "cmsis_os.h"
#include <string.h>
#define KART_SD_IO_MS 500u
static DSTATUS Stat=STA_NOINIT;
extern SD_HandleTypeDef hsd1;
static uint32_t sector_buffer[128] __attribute__((section("KartSDDMA"),aligned(32)));
static volatile uint8_t dma_done,dma_failed;
void BSP_SD_ReadCpltCallback(void){dma_done=1;}
void BSP_SD_WriteCpltCallback(void){dma_done=1;}
void BSP_SD_AbortCallback(void){dma_failed=1;}
void HAL_SD_ErrorCallback(SD_HandleTypeDef* hsd){if(hsd==&hsd1)dma_failed=1;}
static DRESULT failed(void){
    HAL_SD_Abort(&hsd1);
    HAL_NVIC_ClearPendingIRQ(DMA2_Stream3_IRQn);HAL_NVIC_ClearPendingIRQ(DMA2_Stream6_IRQn);
    HAL_NVIC_ClearPendingIRQ(SDMMC1_IRQn);
    Stat=STA_NOINIT;return RES_ERROR;
}
static int completed(uint32_t start){
    while(!dma_done && !dma_failed && HAL_GetTick()-start<KART_SD_IO_MS){
        if(BSP_SD_IsDetected()!=SD_PRESENT)return 0;osDelay(1);
    }
    return dma_done && !dma_failed && hsd1.ErrorCode==HAL_SD_ERROR_NONE;
}
static int ready(uint32_t start){
    while(HAL_GetTick()-start<KART_SD_IO_MS){
        if(BSP_SD_IsDetected()!=SD_PRESENT)return 0;
        if(BSP_SD_GetCardState()==SD_TRANSFER_OK)return 1;
        osDelay(1);
    }return 0;
}
DSTATUS SD_initialize(BYTE lun){(void)lun;Stat=STA_NOINIT;
    if(BSP_SD_IsDetected()!=SD_PRESENT)return Stat|STA_NODISK;
    if(BSP_SD_Init()==MSD_OK && ready(HAL_GetTick()))Stat=0;return Stat;
}
DSTATUS SD_status(BYTE lun){(void)lun;if(BSP_SD_IsDetected()!=SD_PRESENT)Stat=STA_NOINIT|STA_NODISK;return Stat;}
DRESULT SD_read(BYTE lun,BYTE* buff,DWORD sector,UINT count){
    if(!buff||!count)return RES_PARERR;if(SD_status(lun))return RES_NOTRDY;
    uint32_t start=HAL_GetTick();
    for(UINT i=0;i<count;i++){
        if(!ready(start))return failed();
        dma_done=dma_failed=0;SCB_InvalidateDCache_by_Addr((uint32_t*)sector_buffer,512);
        if(BSP_SD_ReadBlocks_DMA(sector_buffer,sector+i,1)!=MSD_OK || !completed(start))return failed();
        SCB_InvalidateDCache_by_Addr((uint32_t*)sector_buffer,512);
        memcpy(buff+i*512,sector_buffer,512);
    }return RES_OK;
}
DRESULT SD_write(BYTE lun,const BYTE* buff,DWORD sector,UINT count){
    if(!buff||!count)return RES_PARERR;if(SD_status(lun))return RES_NOTRDY;
    uint32_t start=HAL_GetTick();
    for(UINT i=0;i<count;i++){
        if(!ready(start))return failed();
        memcpy(sector_buffer,buff+i*512,512);
        SCB_CleanDCache_by_Addr((uint32_t*)sector_buffer,512);dma_done=dma_failed=0;
        if(BSP_SD_WriteBlocks_DMA(sector_buffer,sector+i,1)!=MSD_OK || !completed(start))return failed();
    }return ready(start)?RES_OK:failed();
}
DRESULT SD_ioctl(BYTE lun,BYTE cmd,void* buff){
    if(SD_status(lun))return RES_NOTRDY;
    if(cmd==CTRL_SYNC)return ready(HAL_GetTick())?RES_OK:RES_ERROR;
    if(!buff)return RES_PARERR;BSP_SD_CardInfo info;BSP_SD_GetCardInfo(&info);
    switch(cmd){case GET_SECTOR_COUNT:*(DWORD*)buff=info.LogBlockNbr;return RES_OK;
        case GET_SECTOR_SIZE:*(WORD*)buff=512;return RES_OK;
        case GET_BLOCK_SIZE:*(DWORD*)buff=1;return RES_OK;default:return RES_PARERR;}
}
const Diskio_drvTypeDef SD_Driver={SD_initialize,SD_status,SD_read,SD_write,SD_ioctl};
