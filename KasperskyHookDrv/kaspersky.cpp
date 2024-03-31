#include "kaspersky.hpp"
#include "kernel_modules.hpp"
#include "utils.hpp"

//
// Global pointers for klhk.sys variables.
//
PETHREAD*     hvm_thread_object         = nullptr;
PLONG         hvm_run_requests          = nullptr;
PRKEVENT      hvm_notification_event    = nullptr;
PRKEVENT      hvm_sync_event            = nullptr;
PNTSTATUS     hvm_status                = nullptr;
void***       system_dispatch_array     = nullptr;
unsigned int* ssdt_service_count        = nullptr;
unsigned int* shadow_ssdt_service_count = nullptr;
unsigned int* provider                  = nullptr;

//
// Checks if klhk.sys is loaded.
//
bool kaspersky::is_klhk_loaded( )
{
	return kernel_modules::get_kernel_module_base( L"klhk.sys" ) != 0;
}

//
// Finds required addresses by pattern scanning klhk.sys.
//
bool kaspersky::initialize( )
{
	//
	// Find klhk's hvm thread object.
	//
	auto presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x48\x39\x2D\x00\x00\x00\x00\x89", "xxx????x" );

	if ( !presult )
		return false;

	hvm_thread_object = reinterpret_cast< PETHREAD* >( presult + *reinterpret_cast< int* >( presult + 0x3 ) + 0x7 );

	//
	// Find klhk's hvm run counter.
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\xF0\xFF\x05\x00\x00\x00\x00\x48\x8D\x0D", "xxx????xxx" );

	if ( !presult )
		return false;

	hvm_run_requests = reinterpret_cast< PLONG >( presult + *reinterpret_cast< int* >( presult + 0x3 ) + 0x7 );

	//
	// Find klhk's hvm notification event.
	//
	presult += 0x7;
	hvm_notification_event = reinterpret_cast< PRKEVENT >( presult + *reinterpret_cast< int* >( presult + 0x3 ) + 0x7 );

	//
	// Find klhk's hvm synchronization event.
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x48\x8D\x05\x00\x00\x00\x00\x49\x89\x73", "xxx????xxx" );

	if ( !presult )
		return false;

	hvm_sync_event = reinterpret_cast< PRKEVENT >( presult + *reinterpret_cast< int* >( presult + 0x3 ) + 0x7 );

	//
	// Find klhk's hvm status.
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x8B\x1D\x00\x00\x00\x00\x89", "xx????x" );

	if ( !presult )
		return false;

	hvm_status = reinterpret_cast< PNTSTATUS >( presult + *reinterpret_cast< int* >( presult + 0x2 ) + 0x6 );

	//
	// Find klhk's service table.
	//
	presult = utils::find_pattern_km( L"klhk.sys", "_hvmcode", "\x4C\x8D\x0D\x00\x00\x00\x00\x4D", "xxx????x" );

	if ( !presult )
		return false;

	system_dispatch_array = reinterpret_cast< void*** >( presult + *reinterpret_cast< int* >( presult + 0x3 ) + 0x7 );

	//
	// Find number of services (SSDT).
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x89\x0D\x00\x00\x00\x00\x8B", "xx????x" );

	if ( !presult )
		return false;

	ssdt_service_count = reinterpret_cast< unsigned int* >( presult + *reinterpret_cast< int* >( presult + 0x2 ) + 0x6 );

	//
	// Find number of services (Shadow SSDT).
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x89\x05\x00\x00\x00\x00\x85\xC0", "xx????xx" );

	if ( !presult )
		return false;

	shadow_ssdt_service_count = reinterpret_cast< unsigned int* >( presult + *reinterpret_cast< int* >( presult + 0x2 ) + 0x6 );

	//
	// Find provider data.
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x39\x1D\x00\x00\x00\x00\x75", "xx????x" );

	if ( !presult )
		return false;

	provider = reinterpret_cast< unsigned int* >( presult + *reinterpret_cast< int* >( presult + 2 ) + 0x6 );
	return true;
}

//
// Performs hypervisor initialization.
//
NTSTATUS kaspersky::hvm_init( )
{
	if ( !hvm_thread_object || !( *hvm_thread_object ) ||
		 !hvm_run_requests || !hvm_notification_event ||
		 !hvm_sync_event || !hvm_status ||
		 !provider )
	{
		return STATUS_BAD_DATA;
	}

	//
	// Set provider to random value.
	//
	*provider = 4;

	//
	// Hypervisor initialization.
	//
	_InterlockedIncrement( hvm_run_requests );
	KeResetEvent( hvm_notification_event );
	KeSetEvent( hvm_sync_event, IO_NO_INCREMENT, FALSE );
	KeWaitForSingleObject( hvm_notification_event, Executive, KernelMode, FALSE, nullptr );

	//
	// Get returned status.
	//
	return *hvm_status;
}

//
// Gets the number of services in the SSDT.
//
unsigned int kaspersky::get_svc_count_ssdt( )
{
	return ssdt_service_count ? *ssdt_service_count : 0;
}

//
// Gets the number of services in the Shadow SSDT.
//
unsigned int kaspersky::get_svc_count_shadow_ssdt( )
{
	return shadow_ssdt_service_count ? *shadow_ssdt_service_count : 0;
}

//
// Hooks SSDT functions by changing klhk's service table.
//
bool kaspersky::hook_ssdt_routine( unsigned short index, void* dest, void** poriginal )
{
	if ( !system_dispatch_array || !dest || !poriginal )
		return false;

	//
	// Get SSDT service count.
	//
	const auto svc_count = get_svc_count_ssdt( );

	//
	// Kaspersky's SSDT isn't built / invalid index.
	//
	if ( !svc_count || index >= svc_count )
		return false;

	//
	// Swap entry.
	//
	*poriginal = *system_dispatch_array[ index ];
	*system_dispatch_array[ index ] = dest;
	return true;
}

//
// Unhooks SSDT function.
//
bool kaspersky::unhook_ssdt_routine( unsigned short index, void* original )
{
	if ( !system_dispatch_array || !original )
		return false;

	//
	// Get SSDT service count.
	//
	const auto svc_count = get_svc_count_ssdt( );

	//
	// Kaspersky's SSDT isn't built / invalid index / function not hooked.
	//
	if ( !svc_count || index >= svc_count || *system_dispatch_array[ index ] == original )
		return false;

	//
	// Restore entry.
	//
	*system_dispatch_array[ index ] = original;
	return true;
}

//
// Hooks Shadow SSDT function.
//
bool kaspersky::hook_shadow_ssdt_routine( unsigned short index, void* dest, void** poriginal )
{
	if ( !system_dispatch_array || !dest || !poriginal )
		return false;

	//
	// Get service count for SSDT and Shadow SSDT.
	//
	const auto svc_count = get_svc_count_ssdt( ), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt( );

	//
	// Failed to obtain service count.
	//
	if ( !svc_count || !svc_count_shadow_ssdt )
		return false;

	//
	// Calculate index for dispatch table
	//
	const auto index_dispatch_table = ( index - 0x1000 ) + svc_count;

	//
	// Get dispatch table limit.
	//
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	//
	// Invalid index.
	//
	if ( index_dispatch_table >= dispatch_table_limit )
		return false;

	//
	// Swap entry.
	//
	*poriginal = *system_dispatch_array[ index_dispatch_table ];
	*system_dispatch_array[ index_dispatch_table ] = dest;
	return true;
}

//
// Unhooks Shadow SSDT function.
//
bool kaspersky::unhook_shadow_ssdt_routine( unsigned short index, void* original )
{
	if ( !system_dispatch_array || !original )
		return false;

	//
	// Get service count for SSDT and Shadow SSDT.
	//
	const auto svc_count = get_svc_count_ssdt( ), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt( );

	//
	// Failed to obtain service count.
	//
	if ( !svc_count || !svc_count_shadow_ssdt )
		return false;

	//
	// Calculate index for dispatch table.
	//
	const auto index_dispatch_table = ( index - 0x1000 ) + svc_count;

	//
	// Get dispatch table limit.
	//
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	//
	// Invalid index / function not hooked.
	//
	if ( index_dispatch_table >= dispatch_table_limit || *system_dispatch_array[ index_dispatch_table ] == original )
		return false;

	//
	// Restore entry.
	//
	*system_dispatch_array[ index_dispatch_table ] = original;
	return true;
}

//
// Gets the pointer to a routine in SSDT by its index.
//
void* kaspersky::get_ssdt_routine( unsigned short index )
{
	if ( !system_dispatch_array )
		return nullptr;

	//
	// Get SSDT service count.
	//
	const auto svc_count = get_svc_count_ssdt( );

	//
	// Return the routine's address if the index is valid.
	//
	return ( svc_count && index < svc_count ) ? *system_dispatch_array[ index ] : nullptr;
}

//
// Gets the pointer to a routine in Shadow SSDT by its index.
//
void* kaspersky::get_shadow_ssdt_routine( unsigned short index )
{
	if ( !system_dispatch_array )
		return nullptr;

	//
	// Get service count for SSDT and Shadow SSDT
	//
	const auto svc_count = get_svc_count_ssdt( ), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt( );

	//
	// Failed to obtain service count.
	//
	if ( !svc_count || !svc_count_shadow_ssdt )
		return nullptr;

	//
	// Calculate index for dispatch table.
	//
	const auto index_dispatch_table = ( index - 0x1000 ) + svc_count;

	//
	// Get dispatch table limit.
	//
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	//
	// Return the routine's address if the index is valid.
	//
	return ( index_dispatch_table < dispatch_table_limit ) ? *system_dispatch_array[ index_dispatch_table ] : nullptr;
}