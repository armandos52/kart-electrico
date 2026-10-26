/* ENS-0028: synchronous sector adapter, used only by low-priority SDTask.
 * No DMA/cache aliases or queue of stale DMA completions. Each I/O request has
 * a software deadline. HAL card initialisation retains vendor limits. */
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "cmsis_os.h"
#include <string.h>
#define KART_SD_IO_MS 500u
static DSTATUS Stat=STA_NOINIT;
static uint32_t sector_buffer[128];
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
        if(!ready(start))return RES_ERROR;
        uint32_t spent=HAL_GetTick()-start;if(spent>=KART_SD_IO_MS)return RES_ERROR;
        if(BSP_SD_ReadBlocks(sector_buffer,sector+i,1,KART_SD_IO_MS-spent)!=MSD_OK){Stat=STA_NOINIT;return RES_ERROR;}
        memcpy(buff+i*512,sector_buffer,512);
    }return RES_OK;
}
DRESULT SD_write(BYTE lun,const BYTE* buff,DWORD sector,UINT count){
    if(!buff||!count)return RES_PARERR;if(SD_status(lun))return RES_NOTRDY;
    uint32_t start=HAL_GetTick();
    for(UINT i=0;i<count;i++){
        if(!ready(start))return RES_ERROR;
        uint32_t spent=HAL_GetTick()-start;if(spent>=KART_SD_IO_MS)return RES_ERROR;
        memcpy(sector_buffer,buff+i*512,512);
        if(BSP_SD_WriteBlocks(sector_buffer,sector+i,1,KART_SD_IO_MS-spent)!=MSD_OK){Stat=STA_NOINIT;return RES_ERROR;}
    }return ready(start)?RES_OK:RES_ERROR;
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
