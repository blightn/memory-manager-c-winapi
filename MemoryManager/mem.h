#ifndef _MEM_H_
#define _MEM_H_

#include <Windows.h>

#define MEM_ALLOCATED    0x12345678
#define MEM_FREED        0x87654321
#define INVALID_MEM_SIZE ((SIZE_T)-1)

typedef const VOID* PCVOID;
typedef const BYTE* PCBYTE;

BOOL MemInit(VOID);
VOID MemCleanup(VOID);

PVOID MemAlloc(SIZE_T Size);
PVOID MemReAlloc(PVOID pvMem, SIZE_T NewSize);
SIZE_T MemSize(PVOID pvMem);
VOID MemFree(PVOID pvMem);
VOID MemFreeEx(PVOID pvMem);

#ifdef _DEBUG
static VOID MemCheckCounter(VOID);
static BOOL MemCheckMem(PVOID pvMem);
#endif

#endif // _MEM_H_
