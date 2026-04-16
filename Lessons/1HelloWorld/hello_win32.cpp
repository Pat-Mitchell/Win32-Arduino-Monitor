/// @file hello_win32.cpp
/// @brief Sanity check — Win32 message box hello world.

#include <windows.h>

/// @function WinMain
/// @brief Entry point for Win32 GUI applications (replaces main()).
/// @param hInstance     Handle to the current app instance.
/// @param hPrevInstance Always NULL in modern Windows (legacy remnant).
/// @param lpCmdLine     Command-line args as a single string.
/// @param nCmdShow      How the window should be shown (minimized, normal, etc.).
/// @return Exit status code.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    MessageBox(
        NULL,                    // Parent window (none)
        L"Hello, Win32!",         // Message body
        L"Preflight Check",       // Title bar text
        MB_OK | MB_ICONINFORMATION
    );

    return 0;
}