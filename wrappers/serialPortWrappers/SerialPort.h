/// @file SerialPort.h
/// @brief Warps Win32 serial port I/O for Arduino communication.
///        Owns the port HANDLE, DCB configuration, and read timeouts
///        Designed to be held as a memeber of a Window subclass.

#pragma once
#include <windows.h>

class SerialPort {
  public:
    SerialPort();
    ~SerialPort();

    // ────── ⋆⋅☆⋅⋆ ────────
    // Connection lifecycle
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function Open
    /// @brief Opens and configures a COM port.
    ///        Strips " (in use)" suffix from display names produced by
    ///        ScanComPorts so the ComboBos selection bac be passed directly
    /// @param szPortName Display name
    /// @param iBaudRate Baud rate. Must match Arduino Serial.begin()
    /// @return TRUE on success, FALSE on failure
    BOOL Open(const wchar_t* szPortName, int iBaudRate = 9600);

    /// @function Close
    /// @brief Clses the port handle if open. Safe to call multiple times.
    /// @return void
    void Close();

    /// @function IsOpen
    /// @brief Returns whether the port is currently open
    /// @return TRUE if open.
    BOOL IsOpen() const;

    // ────── ⋆⋅☆⋅⋆ ────────
    // I/O
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function Write
    /// @brief Sends a narrow string command folled by a newline character
    ///        the newline triggers Arduino's readStringUntil('\n')
    /// @param szCmd Null-terminated narrow command string
    /// @return TRUE on success, FALSE if port is closed or write fails
    BOOL Write(const char* szCmd);

    /// @functin Read
    /// @brief Reads available bytes into a caller supplied narrow buffer
    ///        Returns immediately with however many bytes are waiting
    ///        never blocks. Returns 0 if nothing is available
    /// @param arrBuf Destionation buffer
    /// @param iBufLen Buffer capacity in bytes.
    /// @param dwRead Out. Number of bytes actually read
    /// @return TRUE on success, FALSE if port is closed or read fails
    BOOL Read(char* arrBuf, int iBufLen, DWORD& dwRead);

    // ────── ⋆⋅☆⋅⋆ ────────
    // Error info
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function GetLeastErrorCode
    /// @brief Returns the Win32 error code fron the most recent failed call
    /// @return DWORD error code
    DWORD GetLastErrorCode() const;

  private:

    /// @function Configure
    /// @brief Applies DCB baud/parity/stop-bit settings and non-blocking
    ///        COMMTIMEOUTS to an already-open handle
    /// @param iBaudRate Baud rate apply
    /// @return TRUE on success
    BOOL Configure(int iBaudRate);

    HANDLE hPort;
    DWORD dwLastError;
};