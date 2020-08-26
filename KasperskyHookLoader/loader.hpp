#pragma once

#include <Windows.h>
#include <string>

// Helpers for loading drivers by interacting with SCM
//
namespace loader
{
	bool      open_scm       ();
	void      close_scm      ();
	SC_HANDLE create_service ( const std::string& name, const std::string& display_name, const std::string& path );
	bool      delete_service ( const SC_HANDLE handle_service );
	bool      start_service  ( const SC_HANDLE handle_service );
	bool      stop_service   ( const SC_HANDLE handle_service, LPSERVICE_STATUS service_status );
}