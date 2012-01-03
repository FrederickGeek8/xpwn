#pragma once

#include "xpwn_common.h"
#include <hfs/hfsplus.h>

typedef struct HfsContext {
	io_func* io;
	Volume* volume;	
} HfsContext;


#ifdef WIN32 
#define DLLEXPORT __declspec(dllexport)
#define CDECL _cdecl
#else
#define DLLEXPORT __attribute__((visibility("default")))
#define CDECL 
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT void CDECL hfslib_close(HfsContext* ctx);
DLLEXPORT HfsContext* CDECL hfslib_open(const char* fileName);
DLLEXPORT BOOL CDECL hfslib_untar(HfsContext* ctx, const char* tarFile);
DLLEXPORT uint64_t CDECL hfslib_getsize(HfsContext* ctx);
DLLEXPORT BOOL CDECL hfslib_extend(HfsContext* ctx, uint64_t newSize);


#ifdef __cplusplus
}
#endif

