#include "kernel_modules.hpp"

PKLDR_DATA_TABLE_ENTRY kernel_modules::get_ldr_data_by_name( const wchar_t* szmodule )
{
	PKLDR_DATA_TABLE_ENTRY ldr_entry = nullptr;

	UNICODE_STRING mod { };
	RtlInitUnicodeString( &mod, szmodule );

	if ( !PsLoadedModuleList )
		return ldr_entry;

	auto current_ldr_entry = reinterpret_cast< PKLDR_DATA_TABLE_ENTRY >( PsLoadedModuleList->Flink );

	while ( reinterpret_cast< PLIST_ENTRY >( current_ldr_entry ) != PsLoadedModuleList )
	{
		if ( !RtlCompareUnicodeString( &current_ldr_entry->BaseDllName, &mod, TRUE ) )
		{
			ldr_entry = current_ldr_entry;
			break;
		}

		current_ldr_entry = reinterpret_cast< PKLDR_DATA_TABLE_ENTRY >( current_ldr_entry->InLoadOrderLinks.Flink );
	}

	return ldr_entry;
}

uintptr_t kernel_modules::get_kernel_module_base( const wchar_t* szmodule )
{
	const auto* ldr_entry = get_ldr_data_by_name( szmodule );

	return ldr_entry ? reinterpret_cast< uintptr_t >( ldr_entry->DllBase ) : 0;
}