#include "pe.hpp"

PIMAGE_SECTION_HEADER pe::get_section_header( const uintptr_t image_base, const char* section_name )
{
	if ( !image_base || !section_name )
		return nullptr;

	const auto pimage_dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( image_base );
	const auto pimage_nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS64 >( image_base + pimage_dos_header->e_lfanew );

	auto psection = reinterpret_cast< PIMAGE_SECTION_HEADER >( pimage_nt_headers + 1 );

	PIMAGE_SECTION_HEADER psection_hdr = nullptr;

	const auto NumberOfSections = pimage_nt_headers->FileHeader.NumberOfSections;

	for ( auto i = 0; i < NumberOfSections; ++i )
	{
		if ( strstr( psection->Name, section_name ) )
		{
			psection_hdr = psection;
			break;
		}

		++psection;
	}

	return psection_hdr;
}