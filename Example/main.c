#include <Windows.h>
#include <wchar.h>
#include <strsafe.h>

#include "..\MemoryManager\mem.h"

INT wmain(INT Argc, WCHAR* pArgv[], WCHAR* pEnv[])
{
	const PCWSTR pSrc = L"12345";

	DWORD dwSize;
	PWSTR pDst = NULL,
		  pTmp = NULL;

	if (MemInit())
	{
		dwSize = (lstrlenW(pSrc) + 1) * sizeof(WCHAR);

		if (pDst = (PWSTR)MemAlloc(dwSize))
		{
			StringCbCopyW(pDst, dwSize, pSrc);
			dwSize += lstrlenW(pSrc) * sizeof(WCHAR);

			if (pTmp = (PWSTR)MemReAlloc((PVOID)pDst, dwSize))
			{
				pDst = pTmp;
				StringCbCatW(pDst, dwSize, pSrc);

				wprintf(L"Result: %s\nLength: %d chars\nMemory size: %u bytes\n", pDst, lstrlenW(pDst), (DWORD)MemSize((PVOID)pDst));
			}

			MemFree((PVOID)pDst);
			pDst = NULL;
		}

		MemCleanup();
	}

	wprintf(L"\n");
	system("pause");

	return 0;
}
