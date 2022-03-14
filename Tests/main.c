#ifdef NDEBUG
#	error Tests can be run only in debug mode
#endif

#include <Windows.h>
#include <wchar.h>
#include <assert.h>

#include "..\MemoryManager\mem.h"

#pragma comment(lib, "ntdll.lib") // For RtlCompareMemory() on x86.

#define ITERATIONS   128

#define THREAD_COUNT 64 // <= MAXIMUM_WAIT_OBJECTS (64).

typedef const VOID* PCVOID;

static const BYTE g_bMemPattern[] =
{
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
};

static VOID CheckMemBorders(PCBYTE pbMem, SIZE_T Size)
{
	assert(*(PDWORD)(pbMem - sizeof(DWORD)) == MEM_ALLOCATED);
	assert(*(PDWORD)(pbMem + Size)          == MEM_ALLOCATED);
}

static DWORD WINAPI TestThread(PVOID pvParam)
{
	DWORD  dwRes,
	       i;
	PBYTE  pbMem = NULL;
	SIZE_T Size;

	assert(pvParam != NULL);
	dwRes = WaitForSingleObject((HANDLE)pvParam, INFINITE);
	assert(dwRes == WAIT_OBJECT_0);

	for (i = 0; i < ITERATIONS; ++i)
	{
		pbMem = (PBYTE)MemAlloc(sizeof(g_bMemPattern));
		assert(pbMem != NULL);

		Size = MemSize((PVOID)pbMem);
		assert(Size == sizeof(g_bMemPattern));

		RtlCopyMemory((PVOID)pbMem, (PCVOID)g_bMemPattern, sizeof(g_bMemPattern));
		CheckMemBorders(pbMem, Size);

		MemFree((PVOID)pbMem);
		pbMem = NULL;
	}

	return 0;
}

INT wmain(INT Argc, WCHAR* pArgv[], WCHAR* pEnv[])
{
	PBYTE  pbMem  = NULL;
	SIZE_T Size;
	HANDLE hStart = NULL,
	       hThreads[THREAD_COUNT];
	DWORD  i,
	       dwRes;

	assert(ARRAYSIZE(hThreads) <= MAXIMUM_WAIT_OBJECTS);

	// 1 - MemInit()
	assert(MemInit() == TRUE);

	// 2 - MemAlloc()
	pbMem = (PBYTE)MemAlloc(sizeof(g_bMemPattern));
	assert(pbMem != NULL);

	Size = MemSize((PVOID)pbMem);
	assert(Size == sizeof(g_bMemPattern));

	RtlCopyMemory((PVOID)pbMem, (PCVOID)g_bMemPattern, sizeof(g_bMemPattern));

	CheckMemBorders(pbMem, Size);

	// 3 - MemReAlloc()
	pbMem = (PBYTE)MemReAlloc((PVOID)pbMem, sizeof(g_bMemPattern) * 2);
	assert(pbMem != NULL);

	Size = MemSize((PVOID)pbMem);
	assert(Size == sizeof(g_bMemPattern) * 2);

	CheckMemBorders(pbMem, Size);

	assert(RtlCompareMemory((PCVOID)pbMem, (PCVOID)g_bMemPattern, sizeof(g_bMemPattern)) == sizeof(g_bMemPattern));

	// 4 - MemFree()
	MemFree((PVOID)pbMem);
	pbMem = NULL;

	// 5 - Thread-safety
	hStart = CreateEventW(NULL, TRUE, FALSE, NULL);
	assert(hStart != NULL);

	for (i = 0; i < ARRAYSIZE(hThreads); ++i)
	{
		hThreads[i] = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)TestThread, (PVOID)hStart, 0, NULL);
		assert(hThreads[i] != NULL);
	}

	SetEvent(hStart);

	dwRes = WaitForMultipleObjects(ARRAYSIZE(hThreads), hThreads, TRUE, INFINITE);
	assert(dwRes == WAIT_OBJECT_0);

	for (i = 0; i < ARRAYSIZE(hThreads); ++i)
	{
		CloseHandle(hThreads[i]);
		hThreads[i] = NULL;
	}

	CloseHandle(hStart);
	hStart = NULL;

	// 6 - MemCleanup
	MemCleanup();

	wprintf(L"All tests passed\n\n");
	system("pause");

	return 0;
}
