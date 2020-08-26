#include "kaspersky.hpp"
#include "kernel_modules.hpp"
#include "utils.hpp"

// Global pointers for klhk.sys functions/variables
//
f_SetHvmEvent set_hvm_event             = nullptr;
void***       system_dispatch_array     = nullptr;
unsigned int* ssdt_service_count        = nullptr;
unsigned int* shadow_ssdt_service_count = nullptr;
unsigned int* provider                  = nullptr;

// Checks if klhk.sys is loaded
//
bool kaspersky::is_klhk_loaded()
{
	const auto entry = kernel_modules::get_ldr_data_by_name( L"klhk.sys" );

	return entry != nullptr;
}

// Finds required addresses by pattern scanning klhk.sys
//
bool kaspersky::initialize()
{
	// Find SetHvmEvent
	//
	set_hvm_event = reinterpret_cast< f_SetHvmEvent >( utils::find_pattern_km( L"klhk.sys", ".text", "\x48\x83\xEC\x38\x48\x83\x3D",
		"xxxxxxx" ) );

	if ( !set_hvm_event )
		return false;

	// Find klhk's service table
	//
	auto presult = utils::find_pattern_km( L"klhk.sys", "_hvmcode", "\x4C\x8D\x0D\x00\x00\x00\x00\x4D", "xxx????x" );

	if ( !presult )
		return false;

	system_dispatch_array = reinterpret_cast< void*** >( presult + *reinterpret_cast< int* >( presult + 0x3 ) + 0x7 );

	// Find number of services (SSDT)
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x3B\x1D\x00\x00\x00\x00\x73\x56", "xx????xx" );

	if ( !presult )
		return false;

	ssdt_service_count = reinterpret_cast< unsigned int* >( presult + *reinterpret_cast< int* >( presult + 0x2 ) + 0x6 );

	// Find number of services (Shadow SSDT)
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x89\x05\x00\x00\x00\x00\x8B\xFB", "xx????xx" );

	if ( !presult )
		return false;

	shadow_ssdt_service_count = reinterpret_cast< unsigned int* >( presult + *reinterpret_cast< int* >( presult + 0x2 ) + 0x6 );

	// Find provider data
	//
	presult = utils::find_pattern_km( L"klhk.sys", ".text", "\x39\x2D\x00\x00\x00\x00\x75", "xx????x" );

	if ( !presult )
		return false;

	provider = reinterpret_cast< unsigned int* >( presult + *reinterpret_cast< int* >( presult + 2 ) + 0x6 );

	return true;
}

// Perform klhk's hypervisor initialization
//
bool kaspersky::hvm_init()
{
	if ( !provider || !set_hvm_event )
		return false;

	*provider = 4;

	return NT_SUCCESS( set_hvm_event() );
}

// Gets the number of services in the SSDT
//
unsigned int kaspersky::get_svc_count_ssdt()
{
	return ssdt_service_count ? *ssdt_service_count : 0;
}

// Gets the number of services in the Shadow SSDT
//
unsigned int kaspersky::get_svc_count_shadow_ssdt()
{
	return shadow_ssdt_service_count ? *shadow_ssdt_service_count : 0;
}

// Hooks SSDT functions by changing klhk's service table.
//
bool kaspersky::hook_ssdt_routine( unsigned short index, void* dest, void** poriginal )
{
	if ( !system_dispatch_array || !dest || !poriginal )
		return false;

	// Get ssdt service count
	//
	const auto svc_count = get_svc_count_ssdt();

	// Kaspersky's SSDT isn't built/Invalid index
	//
	if ( !svc_count || index >= svc_count )
		return false;

	// Swap entry
	//
	*poriginal = *system_dispatch_array[ index ];
	*system_dispatch_array[ index ] = dest;

	return true;
}

// Unhooks SSDT function
//
bool kaspersky::unhook_ssdt_routine( unsigned short index, void* original )
{
	if ( !system_dispatch_array || !original )
		return false;

	// Get ssdt service count
	//
	const auto svc_count = get_svc_count_ssdt();

	// Kaspersky's SSDT isn't built/Invalid index/Function not hooked
	//
	if ( !svc_count || index >= svc_count || *system_dispatch_array[ index ] == original )
		return false;

	// Restore entry
	//
	*system_dispatch_array[ index ] = original;

	return true;
}

// Hooks shadow SSDT function
//
bool kaspersky::hook_shadow_ssdt_routine( unsigned short index, void* dest, void** poriginal )
{
	if ( !system_dispatch_array || !dest || !poriginal )
		return false;

	// Get service count for ssdt and shadow ssdt
	//
	const auto svc_count = get_svc_count_ssdt(), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt();

	// Failed to obtain service count
	//
	if ( !svc_count || !svc_count_shadow_ssdt )
		return nullptr;

	// Calculate index for dispatch table
	//
	const auto index_dispatch_table = ( index - 0x1000 ) + svc_count;

	// Get dispatch table limit
	//
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	// Invalid index
	//
	if ( index_dispatch_table >= dispatch_table_limit )
		return false;

	// Swap entry
	//
	*poriginal = *system_dispatch_array[ index_dispatch_table ];
	*system_dispatch_array[ index_dispatch_table ] = dest;

	return true;
}

// Unhooks shadow SSDT function
//
bool kaspersky::unhook_shadow_ssdt_routine( unsigned short index, void* original )
{
	if ( !system_dispatch_array || !original )
		return false;

	// Get service count for ssdt and shadow ssdt
	//
	const auto svc_count = get_svc_count_ssdt(), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt();

	// Failed to obtain service count
	//
	if ( !svc_count || !svc_count_shadow_ssdt )
		return nullptr;

	// Calculate index for dispatch table
	//
	const auto index_dispatch_table = ( index - 0x1000 ) + svc_count;

	// Get dispatch table limit
	//
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	// Invalid index/function not hooked
	//
	if ( index_dispatch_table >= dispatch_table_limit || *system_dispatch_array[ index_dispatch_table ] == original )
		return false;

	// Restore entry
	//
	*system_dispatch_array[ index_dispatch_table ] = original;

	return true;
}

// Gets the pointer to a routine in SSDT by its index
//
void* kaspersky::get_ssdt_routine( unsigned short index )
{
	if ( !system_dispatch_array )
		return nullptr;

	// Get ssdt service count
	//
	const auto svc_count = get_svc_count_ssdt();

	// Return the routine's address if the index is valid
	//
	return ( svc_count && index < svc_count ) ? *system_dispatch_array[ index ] : nullptr;
}

// Gets the pointer to a routine in Shadow SSDT by its index
//
void* kaspersky::get_shadow_ssdt_routine( unsigned short index )
{
	if ( !system_dispatch_array )
		return false;

	// Get service count for ssdt and shadow ssdt
	//
	const auto svc_count = get_svc_count_ssdt(), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt();

	// Failed to obtain service count
	//
	if ( !svc_count || !svc_count_shadow_ssdt )
		return nullptr;

	// Calculate index for dispatch table
	//
	const auto index_dispatch_table = ( index - 0x1000 ) + svc_count;

	// Get dispatch table limit
	//
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	// Return the routine's address if the index is valid
	//
	return ( index_dispatch_table < dispatch_table_limit ) ? *system_dispatch_array[ index_dispatch_table ] : nullptr;
}