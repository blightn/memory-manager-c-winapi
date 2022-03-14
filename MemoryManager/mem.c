#include "mem.h"

#ifdef _DEBUG
#	define DEBUG_MSGW(format, ...)  { \
	                                    SYSTEMTIME Time123; \
	                                    WCHAR      Tmp123[512]; \
	                                    INT        Res123; \
	                                    GetLocalTime(&Time123); \
	                                    Res123 = wsprintfW(Tmp123, L"[%02u:%02u:%02u:%03u]: " __FILEW__ L" - " __FUNCTIONW__ L"(): " format L"\n", \
		                                    Time123.wHour, Time123.wMinute, Time123.wSecond, Time123.wMilliseconds, __VA_ARGS__); \
	                                    if (Res123 >= 512) DebugBreak(); \
	                                    OutputDebugStringW(Tmp123); \
	                                }
#else
#	define DEBUG_MSGW(format, ...)  {}
#endif

static HANDLE g_hHeap = NULL;

#ifdef _DEBUG
static volatile LONG64 g_llMemCount;
#endif

// Once the pages are committed, they are not decommitted until the process is terminated or until the heap
// is destroyed by calling the HeapDestroy function.
BOOL MemInit(VOID)
{
	return g_hHeap ? TRUE : (g_hHeap = HeapCreate(0, 0, 0)) != NULL;
}

VOID MemCleanup(VOID)
{
#ifdef _DEBUG
	if (!HeapValidate(g_hHeap, 0, NULL))
	{
		DEBUG_MSGW(L"Invalid heap");
	}
#endif

	if (g_hHeap)
	{
		HeapDestroy(g_hHeap);
		g_hHeap = NULL;
	}
}

#ifndef _DEBUG

PVOID MemAlloc(SIZE_T Size)
{
	PVOID pvMem = NULL;

	if (Size)
	{
		pvMem = (PVOID)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, Size);
	}

	return pvMem;
}

// Retains the original data.
PVOID MemReAlloc(PVOID pvMem, SIZE_T NewSize)
{
	PVOID pvNewMem = NULL;

	if (pvMem && NewSize)
	{
		pvNewMem = (PVOID)HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, (PVOID)pvMem, NewSize);
	}

	return pvNewMem;
}

SIZE_T MemSize(PVOID pvMem)
{
	SIZE_T Size = INVALID_MEM_SIZE;

	if (pvMem)
	{
		Size = HeapSize(g_hHeap, 0, (PCVOID)pvMem);
	}

	return Size;
}

VOID MemFree(PVOID pvMem)
{
	if (pvMem)
	{
		HeapFree(g_hHeap, 0, (PVOID)pvMem);
		pvMem = NULL;
	}
}

// Cleans up memory before freeing.
VOID MemFreeEx(PVOID pvMem)
{
	SIZE_T Size;

	if (pvMem)
	{
		Size = MemSize(pvMem);

		if (Size != INVALID_MEM_SIZE)
		{
			SecureZeroMemory(pvMem, Size);

			HeapFree(g_hHeap, 0, (PVOID)pvMem);
			pvMem = NULL;
		}
	}
}

#else // _DEBUG

PVOID MemAlloc(SIZE_T Size)
{
	PBYTE pbMem = NULL;

	MemCheckCounter();

	if (Size)
	{
		if (pbMem = (PBYTE)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, Size + sizeof(DWORD) * 2))
		{
			*(PDWORD)pbMem                        = MEM_ALLOCATED;
			*(PDWORD)&pbMem[sizeof(DWORD) + Size] = MEM_ALLOCATED;

			InterlockedIncrement64(&g_llMemCount);
		}
		else
			DEBUG_MSGW(L"Memory overflow");

		MemCheckCounter();
	}
	else
		DEBUG_MSGW(L"Invalid memory size");

	return pbMem ? (PVOID)(pbMem + sizeof(DWORD)) : NULL;
}

// Retains the original data.
PVOID MemReAlloc(PVOID pvMem, SIZE_T NewSize)
{
	PBYTE pbNewMem = NULL;

	if (pvMem)
	{
		if (NewSize)
		{
			if (MemCheckMem(pvMem))
			{
				if (pbNewMem = (PBYTE)HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, (PVOID)((PBYTE)pvMem - sizeof(DWORD)), NewSize + sizeof(DWORD) * 2))
				{
					*(PDWORD)pbNewMem                           = MEM_ALLOCATED;
					*(PDWORD)&pbNewMem[sizeof(DWORD) + NewSize] = MEM_ALLOCATED;
				}
				else
					DEBUG_MSGW(L"Memory overflow");
			}
		}
		else
			DEBUG_MSGW(L"Invalid memory size");
	}
	else
		DEBUG_MSGW(L"Memory pointer is NULL");

	return pbNewMem ? (PVOID)(pbNewMem + sizeof(DWORD)) : NULL;
}

SIZE_T MemSize(PVOID pvMem)
{
	SIZE_T Size = INVALID_MEM_SIZE;

	if (pvMem)
	{
		Size = HeapSize(g_hHeap, 0, (PCVOID)((PBYTE)pvMem - sizeof(DWORD)));

		if (Size != INVALID_MEM_SIZE)
		{
			Size -= sizeof(DWORD) * 2;
		}
		else
			DEBUG_MSGW(L"Invalid memory size");
	}
	else
		DEBUG_MSGW(L"Memory pointer is NULL");

	return Size;
}

VOID MemFree(PVOID pvMem)
{
	SIZE_T Size;

	if (pvMem)
	{
		if (MemCheckMem(pvMem))
		{
			Size = MemSize(pvMem);

			if (Size != INVALID_MEM_SIZE)
			{
				*(PDWORD)((PBYTE)pvMem - sizeof(DWORD)) = MEM_FREED;
				*(PDWORD)((PBYTE)pvMem + Size)          = MEM_FREED;

				HeapFree(g_hHeap, 0, (PVOID)((PBYTE)pvMem - sizeof(DWORD)));
				pvMem = NULL;

				MemCheckCounter();
				InterlockedDecrement64(&g_llMemCount);
			}
			else
				DEBUG_MSGW(L"Invalid memory size");
		}
	}
	else
		DEBUG_MSGW(L"Memory pointer is NULL");

	MemCheckCounter();
}

// Cleans up memory before freeing.
VOID MemFreeEx(PVOID pvMem)
{
	SIZE_T Size;

	if (pvMem)
	{
		if (MemCheckMem(pvMem))
		{
			Size = MemSize(pvMem);

			if (Size != INVALID_MEM_SIZE)
			{
				*(PDWORD)((PBYTE)pvMem - sizeof(DWORD)) = MEM_FREED;
				*(PDWORD)((PBYTE)pvMem + Size)          = MEM_FREED;

				SecureZeroMemory(pvMem, Size);

				HeapFree(g_hHeap, 0, (PVOID)((PBYTE)pvMem - sizeof(DWORD)));
				pvMem = NULL;

				MemCheckCounter();
				InterlockedDecrement64(&g_llMemCount);
			}
			else
				DEBUG_MSGW(L"Invalid memory size");
		}
	}
	else
		DEBUG_MSGW(L"Memory pointer is NULL");

	MemCheckCounter();
}

static VOID MemCheckCounter(VOID)
{
	LONG64 llRes;

	if ((llRes = InterlockedAdd64(&g_llMemCount, 0)) < 0)
	{
		DEBUG_MSGW(L"g_llMemCount < 0: %I64d", llRes);
	}
}

static BOOL MemCheckMem(PVOID pvMem)
{
	PBYTE  pbTemp   = NULL;
	SIZE_T Size;
	PDWORD pdwBegin = NULL,
	       pdwEnd   = NULL;
	BOOL   Ok       = FALSE;

	pbTemp = (PBYTE)pvMem - sizeof(DWORD);

	if (HeapValidate(g_hHeap, 0, (PCVOID)pbTemp))
	{
		Size = MemSize(pvMem);

		if (Size != INVALID_MEM_SIZE)
		{
			pdwBegin = (PDWORD)pbTemp;
			pdwEnd   = (PDWORD)((PBYTE)pvMem + Size);

			__try
			{
				if (*pdwBegin != MEM_ALLOCATED || *pdwEnd != MEM_ALLOCATED)
				{
					if (*pdwBegin == MEM_FREED && *pdwEnd == MEM_FREED)
					{
						DEBUG_MSGW(L"Memory is already freed");
					}
					else
						DEBUG_MSGW(L"Memory is corrupted");
				}
				else
					Ok = TRUE;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				DEBUG_MSGW(L"Exception %d in MemCheckMem()", GetExceptionCode());
			}
		}
		else
			DEBUG_MSGW(L"Invalid memory size");
	}
	else
		DEBUG_MSGW(L"Invalid memory");

	return Ok;
}

#endif // _DEBUG
