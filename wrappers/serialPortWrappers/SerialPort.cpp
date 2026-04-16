/// @file SerialPort.cpp
/// @brief Implementation of the SerialPort wrapper

#include "SerialPort.h"

SerialPort::SerialPort()
  : hPort(INVALID_HANDLE_VALUE)
  , dwLastError(0)
{}

SerialPort::~SerialPort() {
  Close();
}

// ────── ⋆⋅☆⋅⋆ ────────
// Connection lifecycle
// ────── ⋆⋅☆⋅⋆ ────────

BOOL SerialPort::Open(const wchar_t* szPortName, int iBaudRate) {
  // Strip " (in use)" suffix if the name came straight from the ComboBox
  wchar_t arrCleanName[32];
  wcsncpy(arrCleanName, szPortName, 31);
  arrCleanName[31] = L'\0';

  wchar_t* pSpace = wcschr(arrCleanName, L' ');
  if(pSpace) *pSpace = L'\0';

  // Build the \\.\COMn path required for all COM ports
  wchar_t arrPortPath[32];
  wsprintf(arrPortPath, L"\\\\.\\%s", arrCleanName);

  hPort = CreateFile(
    arrPortPath,
    GENERIC_READ | GENERIC_WRITE,
    0, // No sharing
    NULL,
    OPEN_EXISTING, // Port must already exist
    0,
    NULL
  );

  if(hPort == INVALID_HANDLE_VALUE) {
    dwLastError = GetLastError();
    return FALSE;
  }

  if(!Configure(iBaudRate)) {
    dwLastError = GetLastError();
    CloseHandle(hPort);
    hPort = INVALID_HANDLE_VALUE;
    return FALSE;
  }

  return TRUE;
}

void SerialPort::Close() {
  if(hPort != INVALID_HANDLE_VALUE) {
    CloseHandle(hPort);
    hPort = INVALID_HANDLE_VALUE;
  }
}

BOOL SerialPort::IsOpen() const {
  return (hPort != INVALID_HANDLE_VALUE);
}

// ────── ⋆⋅☆⋅⋆ ────────
// I/O
// ────── ⋆⋅☆⋅⋆ ────────

BOOL SerialPort::Write(const char* szCmd) {
  if(!IsOpen()) return FALSE;

  // Append newline so Arduino's readStringUntil('\n') fires correctly
  char arrBuf[64];
  int iLen = wsprintfA(arrBuf, "%s\n", szCmd);

  DWORD dwWritten = 0;
  BOOL bResult = WriteFile(hPort, arrBuf, iLen, &dwWritten, NULL);

  if(!bResult) dwLastError = GetLastError();
  return bResult;
}

BOOL SerialPort::Read(char* arrBuf, int iBufLen, DWORD& dwRead) {
  if(!IsOpen()) return FALSE;

  dwRead = 0;
  BOOL bResult = ReadFile(hPort, arrBuf, iBufLen - 1, &dwRead, NULL);

  if(!bResult) dwLastError = GetLastError();
  return bResult;
}

// ────── ⋆⋅☆⋅⋆ ────────
// Error info
// ────── ⋆⋅☆⋅⋆ ────────

DWORD SerialPort::GetLastErrorCode() const {
  return dwLastError;
}

// ────── ⋆⋅☆⋅⋆ ────────
// Private
// ────── ⋆⋅☆⋅⋆ ────────

BOOL SerialPort::Configure(int iBaudRate) {
  // ────── ⋆⋅☆⋅⋆ ────────
  // DCB - Device Control Block
  // GetCommState fills the struct with the port's current settings so we 
  // only override the fields we care about
  // ────── ⋆⋅☆⋅⋆ ────────
  DCB dcb_config = {};
  dcb_config.DCBlength = sizeof(DCB);

  if(!GetCommState(hPort, &dcb_config)) return FALSE;

  dcb_config.BaudRate = iBaudRate;
  dcb_config.ByteSize = 8;
  dcb_config.Parity = NOPARITY;
  dcb_config.StopBits = ONESTOPBIT;

  if(!SetCommState(hPort, &dcb_config)) return FALSE;

  // ────── ⋆⋅☆⋅⋆ ────────
  // COMMTIMEOUTS
  // MAXDWORD + 0 + 0 makes ReadFile return immediately with whatever
  // bytes are in the buffer
  // ────── ⋆⋅☆⋅⋆ ────────
  COMMTIMEOUTS timeouts               = {};
  timeouts.ReadIntervalTimeout        = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant   = 0;

  return SetCommTimeouts(hPort, &timeouts);
}