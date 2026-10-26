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
enum FakeFsOp {FS_MOUNT,FS_OPEN,FS_WRITE,FS_SYNC,FS_CLOSE,FS_UNMOUNT,FS_OP_COUNT};
extern uint32_t tick,fs_delays[FS_OP_COUNT];
extern void (*fs_hook)(FakeFsOp);
inline void fs_simulate(FakeFsOp op){tick+=fs_delays[op];if(fs_hook)fs_hook(op);}
inline FRESULT f_mount(FATFS* fs,const char*,int){if(fs)mounts++;fs_simulate(fs?FS_MOUNT:FS_UNMOUNT);return fs?mount_result:0;}
inline FRESULT f_open(FIL*,const char*,int){opens++;fs_simulate(FS_OPEN);return open_result;}
inline unsigned f_size(FIL*){return (unsigned)file_content.size();}
inline FRESULT f_write(FIL*,const char* data,UINT len,UINT* actual){fs_simulate(FS_WRITE);*actual=short_write?len-1:len;if(!write_result)file_content.append(data,*actual);return write_result;}
inline FRESULT f_sync(FIL*){syncs++;fs_simulate(FS_SYNC);return sync_result;}
inline FRESULT f_close(FIL*){closes++;fs_simulate(FS_CLOSE);return close_result;}
