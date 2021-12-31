#include "kernel_modules.hpp"

uintptr_t kernel_modules::get_kernel_module_base( const wchar_t* szmodule )
{
	if ( !szmodule || !PsLoadedModuleList || !PsLoadedModuleResource )
		return 0;

	UNICODE_STRING module_name{ };
	RtlInitUnicodeString( &module_name, szmodule );

	KeEnterCriticalRegion();
	ExAcquireResourceSharedLite( PsLoadedModuleResource, TRUE );

	uintptr_t module_base = 0;

	for ( const auto* plist_entry = PsLoadedModuleList->Flink;
		  plist_entry != PsLoadedModuleList;
		  plist_entry = plist_entry->Flink )
	{
		const auto* pldr_entry = CONTAINING_RECORD( plist_entry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks );

		if ( !RtlCompareUnicodeString( &pldr_entry->BaseDllName, &module_name, TRUE ) )
		{
			module_base = reinterpret_cast< uintptr_t >( pldr_entry->DllBase );
			break;
		}
	}

	ExReleaseResourceLite( PsLoadedModuleResource );
	KeLeaveCriticalRegion();

	return module_base;
}