#include <iostream>
#include "klhk.hpp"
#include "kasperskyhook.hpp"
#include <TlHelp32.h>

int main()
{
    // Open handle to SCM
    //
    if ( !loader::open_scm() )
    {
        std::cout << "[-] Failed to open handle to SCM!\n";
        return EXIT_FAILURE;
    }

    // Load klhk.sys
    //
    if ( !klhk::load() )
    {
        std::cout << "[-] Failed to load klhk.sys!\n";

        // Do cleanup related to klhk
        //
        klhk::cleanup( true );

        // Close handle to SCM
        //
        loader::close_scm();

        return EXIT_FAILURE;
    }

    // Just to make sure klhk.sys gets Shadow SSDT information. You can change this to whatever you want.
    //
    MessageBoxA( nullptr, "Dummy", "Dummy", MB_OK );

    // Load KasperskyHook.sys
    //
    if ( !kasperskyhook::load() )
    {
        std::cout << "[-] Failed to load KasperskyHook.sys\n";

        // Do cleanup for KasperskyHook
        //
        kasperskyhook::unload();

        // Do cleanup for klhk
        //
        klhk::cleanup( false );

        // Close handle to SCM
        //
        loader::close_scm();

        return EXIT_FAILURE;
    }

    // Wait for user input
    //
    while ( !( GetAsyncKeyState( VK_END ) & 1 ) )
        Sleep( 1 );

    // Unload KasperskyHook.sys
    //
    if ( !kasperskyhook::unload() )
        std::cout << "[-] Failed to unload Kasperskyhook.sys!\n";

    // Do cleanup for klhk
    //
    klhk::cleanup( false );

    // Close handle to SCM
    //
    loader::close_scm();

    return EXIT_SUCCESS;
}