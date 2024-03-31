#include <iostream>
#include "klhk.hpp"
#include "kasperskyhook.hpp"

int main( )
{
    //
    // Open handle to SCM.
    //
    if ( !loader::open_scm( ) )
    {
        std::cout << "[-] Failed to open handle to SCM!\n";
        return EXIT_FAILURE;
    }

    //
    // Load klhk.sys.
    //
    if ( !klhk::load( ) )
    {
        std::cout << "[-] Failed to load klhk.sys!\n";

        //
        // Do cleanup for klhk.sys.
        //
        klhk::cleanup( true );

        //
        // Close handle to SCM.
        //
        loader::close_scm( );
        return EXIT_FAILURE;
    }

    //
    // Just to make sure klhk.sys gets Shadow SSDT information. You can change this to whatever you want.
    //
    MessageBoxA( nullptr, "Dummy MessageBox", "Dummy MessageBox", MB_OK );

    //
    // Load KasperskyHook.sys.
    //
    if ( !kasperskyhook::load( ) )
    {
        std::cout << "[-] Failed to load KasperskyHook.sys\n";

        //
        // Do cleanup for KasperskyHook.sys.
        //
        kasperskyhook::unload( );

        //
        // Do cleanup for klhk.sys.
        //
        klhk::cleanup( false );

        //
        // Close handle to SCM.
        //
        loader::close_scm( );
        return EXIT_FAILURE;
    }

    //
    // Log progress.
    //
    std::cout << "[+] Successfully loaded KasperskyHook! Press END to unload.\n";

    //
    // Wait for user input.
    //
    while ( !( GetAsyncKeyState( VK_END ) & 1 ) )
        Sleep( 10 );

    //
    // Unload KasperskyHook.sys.
    //
    if ( !kasperskyhook::unload( ) )
        std::cout << "[-] Failed to unload Kasperskyhook.sys!\n";

    //
    // Do cleanup for klhk.sys.
    //
    klhk::cleanup( false );

    //
    // Close handle to SCM.
    //
    loader::close_scm( );

    //
    // Return success.
    //
    return EXIT_SUCCESS;
}