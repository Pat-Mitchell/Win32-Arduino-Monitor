/// @file arduino_detect.cpp
/// @brief Detects whether an Arduino is available on COM3 using Win32 serial.

#include <windows.h>

/// @function WinMain
/// @description Opens as a file handle. Reports success or failure via
///              a message box and immediately closes the port.
/// @param hInstance     Handle to thie app instance.
/// @param hPrevInstance Legacy - always NULL
/// @param lpCmdLine     Command-line string
/// @param nCmdShow      Initial window state.
/// @return Exit code
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // ────── ⋆⋅☆⋅⋆ ────────
  // Attempt to open COM3
  // 
  // CreateFile is Win32's universal open/connent call. For serial ports:
  //   - GENERIC_READ | GENERIC_WRITE - We want both directions
  //   - 0                            - No sharing (ports can't be shared)
  //   - OPEN_EXISTING                - port must already exist. Don't create.
  //   - FILE_ATTRIBUTE_NORMAL        - no special file attributes
  //   - FILE_FLAG_OVERLAPPED would enable async I/O - we skip it for now
  // 
  // The L"\\\\.\\" prefix is required for COM ports above COM9 and is
  // harmless on lower ones. Good habit to always include it.
  // ────── ⋆⋅☆⋅⋆ ────────
  HANDLE hPort = CreateFile(
    L"\\\\.\\COM3",             // Port path
    GENERIC_READ | GENERIC_WRITE,
    0,                          // no sharing
    NULL,                       // Default security attributes
    OPEN_EXISTING,              // Port must already exist
    0,                          // No special flags
    NULL                        // No template file
  );

  if(hPort == INVALID_HANDLE_VALUE) {
    // GetLastError() returns a Win32 error code. The most common ones:
    // ERROR_FILE_NOT_FOUND (2) - COM3 doesn't exist (board not plugged in).
    // ERROR_ACCESS_DENIED (5)  - port is open in another program.
    DWORD dwErr = GetLastError();

    wchar_t arrMsg[128];
    wsprintf(arrMsg, L"Arduino not detected on COM3. \n\nError code %lu", dwErr);

    MessageBox(NULL, arrMsg, L"Detection Failed", MB_OK | MB_ICONERROR);
    return 1;
  }

  // Port opened successfully - Arduino is present.
  MessageBox(NULL,
             L"Arduino detected on COM3.",
             L"Detection OK",
             MB_OK | MB_ICONINFORMATION
  );

  // Always close the handle when done. Leaving it open blocks other
  // Programs (including the Arduino IDE) from accessing the port.
  CloseHandle(hPort);

  return 0;
}