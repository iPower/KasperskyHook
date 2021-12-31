#pragma once

#include <ntifs.h>
#include <ntimage.h>
#include <windef.h>

namespace pe
{
	PIMAGE_SECTION_HEADER get_section_header( const uintptr_t image_base, const char* section_name );
}