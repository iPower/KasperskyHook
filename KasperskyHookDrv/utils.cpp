#include "utils.hpp"
#include "kernel_modules.hpp"
#include "pe.hpp"

bool data_compare( const char* pdata, const char* bmask, const char* szmask )
{
	for ( ; *szmask; ++szmask, ++pdata, ++bmask )
	{
		if ( *szmask == 'x' && *pdata != *bmask )
			return false;
	}

	return !*szmask;
}

uintptr_t utils::find_pattern( const uintptr_t base, const size_t size, const char* bmask, const char* szmask )
{
	for ( size_t i = 0; i < size; ++i )
		if ( data_compare( reinterpret_cast< const char* >( base + i ), bmask, szmask ) )
			return base + i;

	return 0;
}

uintptr_t utils::find_pattern_km( const wchar_t* szmodule, const char* szsection, const char* bmask, const char* szmask )
{
	if ( !szmodule || !szsection || !bmask || !szmask )
		return 0;

	const auto* pldr_entry = kernel_modules::get_ldr_data_by_name( szmodule );

	if ( !pldr_entry )
		return 0;

	const auto  module_base = reinterpret_cast< uintptr_t >( pldr_entry->DllBase );
	const auto* psection    = pe::get_section_header( reinterpret_cast< uintptr_t >( pldr_entry->DllBase ), szsection );

	return psection ? find_pattern( module_base + psection->VirtualAddress, psection->VirtualSize, bmask, szmask ) : 0;
}