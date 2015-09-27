#include "injectory/injector_helper.hpp"
#include "injectory/process.hpp"
#include "injectory/module.hpp"
#include <Psapi.h>

BOOL
EnablePrivilegeW(
	LPCWSTR lpPrivilegeName,
	BOOL bEnable
	)
{
	HANDLE				hToken				= (HANDLE)0;
	HANDLE				hProcess			= (HANDLE)0;
	BOOL				bRet				= FALSE;
	LUID				luid				= {0};
	TOKEN_PRIVILEGES	token_privileges	= {1};

	__try
	{
		// Open current process token with adjust rights
		if(!OpenProcessToken(GetCurrentProcess(),
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken))
		{
			PRINT_ERROR_MSGA("Could not open process token.");
			__leave;
		}

		if(!LookupPrivilegeValueW((LPCWSTR)0, lpPrivilegeName, &luid))
		{
			PRINT_ERROR_MSGW(L"Could not look up privilege value for \"%s\".",
				lpPrivilegeName);
			__leave;
		}
		if(luid.LowPart == 0 && luid.HighPart == 0)
		{
			PRINT_ERROR_MSGW(L"Could not get LUID for \"%s\".", lpPrivilegeName);
			__leave;
		}

		// Set the privileges we need
		token_privileges.Privileges[0].Luid = luid;
		token_privileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

		// Apply the adjusted privileges
		if(!AdjustTokenPrivileges(hToken, FALSE, &token_privileges, sizeof(TOKEN_PRIVILEGES),
			(PTOKEN_PRIVILEGES)0, (PDWORD)0))
		{
			PRINT_ERROR_MSGA("Could not adjust token privileges.");
			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		if(hToken)
		{
			CloseHandle(hToken);
		}
	}

	return bRet;
}

void Process::listModules()
{
	MEMORY_BASIC_INFORMATION mem_basic_info = { 0 };
	SYSTEM_INFO sys_info = getSystemInfo();

	printf("BASE\t\t SIZE\t\t  MODULE\n\n");

	for(SIZE_T mem = 0; mem < (SIZE_T)sys_info.lpMaximumApplicationAddress; mem += mem_basic_info.RegionSize)
	{
		PVOID ab = mem_basic_info.AllocationBase;
		mem_basic_info = memBasicInfo((LPCVOID)mem);
		if (ab == mem_basic_info.AllocationBase)
			continue;

		wstring ntMappedFileName = getInjected((HMODULE)mem_basic_info.AllocationBase).mappedFilename(false);

		if (!ntMappedFileName.empty())
		{
			IMAGE_NT_HEADERS nt_header = {0};
			IMAGE_DOS_HEADER dos_header = {0};
			SIZE_T NumBytesRead = 0;
			LPVOID lpNtHeaderAddress = 0;
			//WCHAR *pModuleName = (WCHAR)0;

			//pModuleName = wcsrchr(ntMappedFileName, '\\');
			//if(!pModuleName)
			//{
			//	return;
			//}
			//++pModuleName;
				
			if(ReadProcessMemory(handle(), mem_basic_info.AllocationBase, &dos_header,
				sizeof(IMAGE_DOS_HEADER), &NumBytesRead) &&
				NumBytesRead == sizeof(IMAGE_DOS_HEADER))
			{
				lpNtHeaderAddress = (LPVOID)( (DWORD_PTR)mem_basic_info.AllocationBase +
					dos_header.e_lfanew );

				if(ReadProcessMemory(handle(), lpNtHeaderAddress, &nt_header,
					sizeof(IMAGE_NT_HEADERS), &NumBytesRead) &&
					NumBytesRead == sizeof(IMAGE_NT_HEADERS))
				{
					wprintf(L"0x%p, %.1f kB, %s\n",
						mem_basic_info.AllocationBase,
						nt_header.OptionalHeader.SizeOfImage/1024.0,
						ntMappedFileName.c_str());
				}
			}
		}
	}
}

