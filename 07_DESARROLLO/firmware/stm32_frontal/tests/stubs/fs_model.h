#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include <set>
typedef unsigned UINT;typedef uint32_t DWORD;typedef uint32_t FSIZE_t;typedef int FRESULT;
struct FATFS{unsigned csize=8;};struct FIL{FSIZE_t pos=0;std::string name;};
#define FR_OK 0
#define FR_DISK_ERR 1
#define FR_INT_ERR 2
#define FR_EXIST 8
#define FA_CREATE_NEW 4
#define FA_OPEN_APPEND 0x30
#define FA_WRITE 2
#define FA_READ 1
extern int mount_result,open_result,write_result,sync_result,close_result,short_write,read_result,seek_result;
extern int mounts,opens,closes,syncs;extern std::string file_content;
inline std::map<std::string,std::string> fs_files;inline std::set<std::string> fs_dirs;
inline bool storage_present=true;
enum FakeFsOp {FS_MOUNT,FS_OPEN,FS_WRITE,FS_SYNC,FS_CLOSE,FS_UNMOUNT,FS_READ,FS_SEEK,FS_OP_COUNT};
extern uint32_t tick,fs_delays[FS_OP_COUNT];extern void (*fs_hook)(FakeFsOp);
inline void fs_simulate(FakeFsOp op){tick+=fs_delays[op];if(fs_hook)fs_hook(op);}
inline FRESULT f_mount(FATFS* fs,const char*,int){if(fs)mounts++;fs_simulate(fs?FS_MOUNT:FS_UNMOUNT);return fs?mount_result:0;}
inline FRESULT f_mkdir(const char* p){return fs_dirs.insert(p).second?FR_OK:FR_EXIST;}
inline FRESULT f_getfree(const char*,DWORD* count,FATFS** fs){static FATFS fake;*count=12345;*fs=&fake;return 0;}
inline FRESULT f_open(FIL* f,const char* p,int mode){opens++;fs_simulate(FS_OPEN);if(open_result)return open_result;if((mode&FA_CREATE_NEW)&&fs_files.count(p))return FR_EXIST;f->name=p;f->pos=(mode&FA_OPEN_APPEND)==FA_OPEN_APPEND?(FSIZE_t)fs_files[p].size():0;fs_files[p];return 0;}
inline unsigned f_size(FIL* f){return (unsigned)fs_files[f->name].size();}
inline FRESULT f_write(FIL* f,const char* data,UINT len,UINT* actual){fs_simulate(FS_WRITE);*actual=short_write?len-1:len;if(!write_result){fs_files[f->name].replace(f->pos,*actual,data,*actual);f->pos+=*actual;file_content=fs_files[f->name];}return write_result;}
inline FRESULT f_lseek(FIL* f,FSIZE_t pos){fs_simulate(FS_SEEK);if(seek_result)return seek_result;if(pos>fs_files[f->name].size())return FR_INT_ERR;f->pos=pos;return 0;}
inline FRESULT f_read(FIL* f,void* data,UINT len,UINT* actual){fs_simulate(FS_READ);*actual=0;if(read_result)return read_result;auto& text=fs_files[f->name];*actual=(UINT)((text.size()-f->pos)<len?text.size()-f->pos:len);memcpy(data,text.data()+f->pos,*actual);f->pos+=*actual;return 0;}
inline FRESULT f_sync(FIL*){syncs++;fs_simulate(FS_SYNC);return sync_result;}
inline FRESULT f_close(FIL*){closes++;fs_simulate(FS_CLOSE);return close_result;}
