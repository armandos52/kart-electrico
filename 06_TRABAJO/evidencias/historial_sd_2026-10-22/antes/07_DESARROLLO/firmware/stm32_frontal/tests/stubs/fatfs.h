#pragma once
#include <stdint.h>
#include <string>
typedef unsigned UINT;typedef uint32_t FSIZE_t;typedef int FRESULT;struct FATFS{};struct FIL{FSIZE_t pos;};
#define FR_OK 0
#define FR_DISK_ERR 1
#define FR_INT_ERR 2
#define FA_OPEN_APPEND 0x30
#define FA_WRITE 2
#define FA_READ 1
extern int mount_result,open_result,write_result,sync_result,close_result,short_write;
extern int read_result,seek_result;
extern int mounts,opens,closes,syncs;extern std::string file_content;
enum FakeFsOp {FS_MOUNT,FS_OPEN,FS_WRITE,FS_SYNC,FS_CLOSE,FS_UNMOUNT,FS_READ,FS_SEEK,FS_OP_COUNT};
extern uint32_t tick,fs_delays[FS_OP_COUNT];
extern void (*fs_hook)(FakeFsOp);
inline void fs_simulate(FakeFsOp op){tick+=fs_delays[op];if(fs_hook)fs_hook(op);}
inline FRESULT f_mount(FATFS* fs,const char*,int){if(fs)mounts++;fs_simulate(fs?FS_MOUNT:FS_UNMOUNT);return fs?mount_result:0;}
inline FRESULT f_open(FIL* file,const char*,int){opens++;fs_simulate(FS_OPEN);file->pos=(FSIZE_t)file_content.size();return open_result;}
inline unsigned f_size(FIL*){return (unsigned)file_content.size();}
inline FRESULT f_write(FIL* file,const char* data,UINT len,UINT* actual){fs_simulate(FS_WRITE);*actual=short_write?len-1:len;if(!write_result){file_content.replace(file->pos,*actual,data,*actual);file->pos+=*actual;}return write_result;}
inline FRESULT f_lseek(FIL* file,FSIZE_t pos){fs_simulate(FS_SEEK);if(seek_result)return seek_result;if(pos>file_content.size())return FR_INT_ERR;file->pos=pos;return FR_OK;}
inline FRESULT f_read(FIL* file,void* data,UINT len,UINT* actual){
    fs_simulate(FS_READ);*actual=0;if(read_result)return read_result;
    const size_t available=file_content.size()-file->pos;*actual=(UINT)(available<len?available:len);
    memcpy(data,file_content.data()+file->pos,*actual);file->pos+=*actual;return FR_OK;
}
inline FRESULT f_sync(FIL*){syncs++;fs_simulate(FS_SYNC);return sync_result;}
inline FRESULT f_close(FIL*){closes++;fs_simulate(FS_CLOSE);return close_result;}
