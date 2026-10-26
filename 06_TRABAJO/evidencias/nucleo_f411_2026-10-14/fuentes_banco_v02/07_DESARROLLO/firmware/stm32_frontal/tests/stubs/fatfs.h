#pragma once
#include <stdint.h>
#include <string>
typedef unsigned UINT;typedef int FRESULT;struct FATFS{};struct FIL{};
#define FR_OK 0
#define FR_DISK_ERR 1
#define FR_INT_ERR 2
#define FA_OPEN_APPEND 1
#define FA_WRITE 2
extern int mount_result,open_result,write_result,sync_result,close_result,short_write;
extern int mounts,opens,closes,syncs;extern std::string file_content;
inline FRESULT f_mount(FATFS* fs,const char*,int){if(fs)mounts++;return fs?mount_result:0;}
inline FRESULT f_open(FIL*,const char*,int){opens++;return open_result;}
inline unsigned f_size(FIL*){return (unsigned)file_content.size();}
inline FRESULT f_write(FIL*,const char* data,UINT len,UINT* actual){*actual=short_write?len-1:len;if(!write_result)file_content.append(data,*actual);return write_result;}
inline FRESULT f_sync(FIL*){syncs++;return sync_result;}
inline FRESULT f_close(FIL*){closes++;return close_result;}
