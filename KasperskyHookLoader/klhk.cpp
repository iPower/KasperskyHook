#include "klhk.hpp"

SC_HANDLE handle_klhk_svc = nullptr;
HKEY      hparameters_key = nullptr;

// Loads klhk.sys
//
bool klhk::load()
{
    // Get system32 directory
    //
    char buf[ MAX_PATH ]{ };
    
    if ( !GetSystemDirectoryA( buf, sizeof( buf ) ) )
        return false;

    // Build klhk.sys path
    //
    const auto path = std::string( buf ) + "\\drivers\\klhk.sys";

	// Create klhk service
	//
	handle_klhk_svc = loader::create_service( "klhk", 
                                              "Kaspersky Lab service driver", 
                                              path );

	// Failed to create service
	//
	if ( !handle_klhk_svc )
		return false;

    // Create Parameters subkey
    //
    auto error_code = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                      TEXT( "System\\CurrentControlSet\\Services\\klhk\\Parameters" ),
                                      0,
                                      nullptr,
                                      0,
                                      KEY_ALL_ACCESS,
                                      nullptr,
                                      &hparameters_key,
                                      nullptr );

    // Failed to create Parameters key
    //
    if ( error_code != ERROR_SUCCESS )
    {
        // Delete service
        //
        loader::delete_service( handle_klhk_svc );

        return false;
    }

    // Setup UseHvm parameter
    //
    DWORD use_hvm = 1;

    error_code = RegSetValueEx( hparameters_key,
                                TEXT( "UseHvm" ),
                                0,
                                REG_DWORD,
                                reinterpret_cast< const BYTE* >( &use_hvm ),
                                sizeof( use_hvm ) );

    // Failed to set up parameter
    //
    if ( error_code != ERROR_SUCCESS )
    {
        // Delete Parameters key
        //
        RegDeleteKey( HKEY_LOCAL_MACHINE, TEXT( "System\\CurrentControlSet\\Services\\klhk\\Parameters" ) );
        RegCloseKey( hparameters_key );

        // Delete service
        //
        loader::delete_service( handle_klhk_svc );

        return false;
    }

    // Load klhk.sys
    //
    const auto success = loader::start_service( handle_klhk_svc );

    // Failed to load klhk.sys
    //
    if ( !success )
    {
        // Delete UseHvm value and Parameters key
        //
        RegDeleteValue( hparameters_key, TEXT( "UseHvm" ) );
        RegDeleteKey( HKEY_LOCAL_MACHINE, TEXT( "System\\CurrentControlSet\\Services\\klhk\\Parameters" ) );
        RegCloseKey( hparameters_key );

        // Delete service
        //
        loader::delete_service( handle_klhk_svc );
    }

    return success;
}

// Close handles to resources
//
void klhk::cleanup( bool delete_service )
{
    if ( hparameters_key )
    {
        // Close handle to parameters key
        //
        RegCloseKey( hparameters_key );
    }

    if ( handle_klhk_svc )
    {
        // Mark service for deletion
        //
        if ( delete_service )
            loader::delete_service( handle_klhk_svc );

        // Close handle to klhk service
        //
        CloseServiceHandle( handle_klhk_svc );
    }
}