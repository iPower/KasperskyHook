#include "kaspersky.hpp"
#include "hooks.hpp"
#include "utils.hpp"

// DriverUnload routine
//
DRIVER_UNLOAD DriverUnload;

// This is just a demo, don't hardcode SSDT indexes.
//
constexpr unsigned short NtCreateFile_index = 0x55;

// Driver initialization
//
EXTERN_C NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath )
{
	UNREFERENCED_PARAMETER( RegistryPath );

	// Setup DriverUnload routine so this driver can unload safely
	//
	DriverObject->DriverUnload = DriverUnload;

	// Initialization related to klhk.sys
	//
	if ( !kaspersky::is_klhk_loaded() || !kaspersky::initialize() )
		return STATUS_UNSUCCESSFUL;

	// Initialize hypervisor
	//
	if ( !kaspersky::hvm_init() )
		return STATUS_UNSUCCESSFUL;

	// SSDT hooks
	//
	const auto success = kaspersky::hook_ssdt_routine( NtCreateFile_index, hooks::hk_NtCreateFile, 
		reinterpret_cast< void** >( &o_NtCreateFile ) );

	return success ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

// Driver cleanup. Undo hooks
//
void DriverUnload( PDRIVER_OBJECT DriverObject )
{
	UNREFERENCED_PARAMETER( DriverObject );

	// Unhook NtCreateFile if klhk.sys is still loaded
	//
	if ( kaspersky::is_klhk_loaded() )
	{
		if ( !kaspersky::unhook_ssdt_routine( NtCreateFile_index, o_NtCreateFile ) )
			log( "Failed to unhook NtCreateFile. Probably not hooked." );

		// Delay execution to make sure no thread is executing our hook. (Better solution: implement a synchronization mechanism)
		//
		LARGE_INTEGER LargeInteger { };
		LargeInteger.QuadPart = -10000000;

		KeDelayExecutionThread( KernelMode, FALSE, &LargeInteger );
	}
}