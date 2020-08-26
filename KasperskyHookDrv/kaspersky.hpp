#pragma once

#include <ntifs.h>
#include <windef.h>

// SetHvmEvent
// It's the function responsible for setting a synchronization event that klhk waits for before initializing the HV
//
using f_SetHvmEvent = NTSTATUS( * )( );

// Functions
//
namespace kaspersky
{
	bool         is_klhk_loaded             ();
	bool         initialize                 ();
	bool         hvm_init                   ();
	unsigned int get_svc_count_ssdt         ();
	unsigned int get_svc_count_shadow_ssdt  ();
	bool         hook_ssdt_routine          ( unsigned short index, void* dest, void** poriginal );
	bool         unhook_ssdt_routine        ( unsigned short index, void* original );
	bool         hook_shadow_ssdt_routine   ( unsigned short index, void* dest, void** poriginal );
	bool         unhook_shadow_ssdt_routine ( unsigned short index, void* original );
	void*        get_ssdt_routine           ( unsigned short index );
	void*        get_shadow_ssdt_routine    ( unsigned short index );
}