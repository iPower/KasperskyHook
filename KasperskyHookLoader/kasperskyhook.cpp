#include "kasperskyhook.hpp"

SC_HANDLE handle_kasperskyhook_svc = nullptr;

// Loads KasperskyHook.sys
//
bool kasperskyhook::load()
{
	// Get current directory
	//
	char buf[ MAX_PATH ]{ };
	GetCurrentDirectoryA( sizeof( buf ), buf );

	// Build KasperskyHook.sys path
	//
	const auto path = std::string( buf ) + "\\KasperskyHookDrv.sys";

	// Create KasperskyHook service
	//
	handle_kasperskyhook_svc = loader::create_service( "KasperskyHook", "KasperskyHook", path );

	// Load KasperskyHook.sys
	//
	return handle_kasperskyhook_svc ? loader::start_service( handle_kasperskyhook_svc ) : false;
}

// Unloads KasperskyHook.sys
//
bool kasperskyhook::unload()
{
	SERVICE_STATUS svc_status { };

	// Unload KasperskyHook.sys
	//
	bool success = loader::stop_service( handle_kasperskyhook_svc, &svc_status );

	// Service not started
	//
	if ( !success && GetLastError() == ERROR_SERVICE_NOT_ACTIVE )
		success = true;

	// Delete KasperskyHook service
	//
	return success ? loader::delete_service( handle_kasperskyhook_svc ) : false;
}