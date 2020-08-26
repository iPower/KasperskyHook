#include "hooks.hpp"
#include "utils.hpp"

// Function pointers to original functions
//
f_NtCreateFile o_NtCreateFile = nullptr;

// Our hook to NtCreateFile. Useless hook for now
//
NTSTATUS hooks::hk_NtCreateFile
(
	PHANDLE            FileHandle, 
	ACCESS_MASK        DesiredAccess, 
	POBJECT_ATTRIBUTES ObjectAttributes, 
	PIO_STATUS_BLOCK   IoStatusBlock, 
	PLARGE_INTEGER     AllocationSize, 
	ULONG              FileAttributes, 
	ULONG              ShareAccess, 
	ULONG              CreateDisposition, 
	ULONG              CreateOptions, 
	PVOID              EaBuffer, 
	ULONG              EaLength )
{
	if ( ObjectAttributes && ObjectAttributes->ObjectName && ObjectAttributes->ObjectName->Buffer )
	{
		const auto name = ObjectAttributes->ObjectName->Buffer;

		// Deny access to any file called "you_wont_open_this.txt"
		//
		if ( wcsstr( name, L"you_wont_open_this.txt" ) )
			return STATUS_ACCESS_DENIED;
	}

	// Call the original function
	//
	return o_NtCreateFile( FileHandle, 
						   DesiredAccess, 
						   ObjectAttributes, 
						   IoStatusBlock, 
						   AllocationSize, 
						   FileAttributes, 
						   ShareAccess, 
						   CreateDisposition, 
						   CreateOptions, 
						   EaBuffer, 
						   EaLength );
}