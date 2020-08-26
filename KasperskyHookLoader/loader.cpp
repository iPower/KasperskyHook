#include "loader.hpp"

SC_HANDLE handle_scm = nullptr;

// Opens a handle to SCM
//
bool loader::open_scm()
{
    handle_scm = OpenSCManager( nullptr, nullptr, SC_MANAGER_CREATE_SERVICE );

    return handle_scm != nullptr;
}

// Closes handle to SCM
//
void loader::close_scm()
{
    CloseServiceHandle( handle_scm );
}

// Creates/opens a service
//
SC_HANDLE loader::create_service( const std::string& name, const std::string& display_name, const std::string& path )
{
    // Create service
    //
    auto hsvc = CreateServiceA( handle_scm,
                                name.c_str(),
                                display_name.c_str(),
                                SERVICE_ALL_ACCESS,
                                SERVICE_KERNEL_DRIVER,
                                SERVICE_DEMAND_START,
                                SERVICE_ERROR_NORMAL,
                                path.c_str(),
                                nullptr,
                                nullptr,
                                nullptr,
                                nullptr,
                                nullptr );

    // Service already exists
    //
    if ( !hsvc && GetLastError() == ERROR_SERVICE_EXISTS )
    {
        // Open handle to service
        //
        hsvc = OpenServiceA( handle_scm, name.c_str(), SERVICE_ALL_ACCESS );
    }

    return hsvc;
}

// Deletes a service
//
bool loader::delete_service( const SC_HANDLE handle_service )
{
    // Mark the service for deletion
    //
    const auto success = static_cast< bool >( DeleteService( handle_service ) );

    // Invalid handle or access rights
    //
    if ( !success && GetLastError() != ERROR_SERVICE_MARKED_FOR_DELETE )
        return false;

    // Close handle to service
    //
    return static_cast< bool >( CloseServiceHandle( handle_service ) );
}

// Starts a service
//
bool loader::start_service( const SC_HANDLE handle_service )
{
    // Start service
    //
    const auto success = static_cast< bool >( StartService( handle_service, 0, nullptr ) );

    return success || GetLastError() == ERROR_SERVICE_ALREADY_RUNNING;
}

// Stops a service
//
bool loader::stop_service( SC_HANDLE handle_service, LPSERVICE_STATUS service_status )
{
    // Stop service
    //
    return static_cast< bool >( ControlService( handle_service, SERVICE_CONTROL_STOP, service_status ) );
}