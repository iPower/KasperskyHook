#pragma once

#include <ntifs.h>

typedef struct _KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY	 InLoadOrderLinks;
	void		 *ExceptionTable;
	unsigned int	 ExceptionTableSize;
	void		 *GpValue;
	void		 *NonPagedDebugInfo;
	void	         *DllBase;
	void		 *EntryPoint;
	unsigned int	 SizeOfImage;
	UNICODE_STRING	 FullDllName;
	UNICODE_STRING	 BaseDllName;
	unsigned int	 Flags;
	unsigned __int16 LoadCount;
	unsigned __int16 u1;
	void	         *SectionPointer;
	unsigned int	 CheckSum;
	unsigned int	 CoverageSectionSize;
	void		 *CoverageSection;
	void		 *LoadedImports;
	void		 *Spare;
	unsigned int	 SizeOfImageNotRounded;
	unsigned int	 TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;


namespace kernel_modules
{
	PKLDR_DATA_TABLE_ENTRY get_ldr_data_by_name   ( const wchar_t* szmodule );
	uintptr_t              get_kernel_module_base ( const wchar_t* szmodule );
}

EXTERN_C PLIST_ENTRY PsLoadedModuleList;