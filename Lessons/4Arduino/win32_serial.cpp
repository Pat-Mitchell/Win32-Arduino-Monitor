/// @file win32_serial.cpp
/// @brief COM port selector + live Arduino serial monitor.

#include <windows.h>

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_COMBO_PORT     101
#define ID_BTN_CONNECT    102
#define ID_BTN_DISCONNECT 103
#define ID_EDIT_OUTPUT    104
#define ID_BTN_LED        105
#define ID_TIMER_POLL     1

// ────── ⋆⋅☆⋅⋆ ────────
// Serial port config. Must match Arduino sketch
// ────── ⋆⋅☆⋅⋆ ────────
#define BAUD_RATE 9600
#define POLL_MS   100   // How often WM_TIMER polls for new bytes (ms)
#define READ_BUF  256   // Max bytes read per timer tick

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/// @function ScanComPorts
/// @description Probes COM1-COM20 via CreateFile and adds available ports to a
///              COMBOBOX. Ports that exist but are busy are labed "(in-use)".
/// @param hwnd_combo Handle to the target COMBOBOX control
/// @return void
void ScanComPorts(HWND hwnd_combo) {
  SendMessage(hwnd_combo, CB_RESETCONTENT, 0, 0); // Clear existing items

  for(int i = 1; i <= 20; i++) {
    wchar_t arrPortPath[16];
    wsprintf(arrPortPath, L"\\\\.\\COM%d", i);

    HANDLE hTest = CreateFile(arrPortPath,
                              GENERIC_READ | GENERIC_WRITE,
                            0, NULL, OPEN_EXISTING, 0, NULL);
    
    if(hTest != INVALID_HANDLE_VALUE) {
      // Port exists and is free
      wchar_t arrDisplayName[8];
      wsprintf(arrDisplayName, L"COM%d", i);
      SendMessage(hwnd_combo, CB_ADDSTRING, 0, (LPARAM)arrDisplayName);
      CloseHandle(hTest);
    } else if(GetLastError() == ERROR_ACCESS_DENIED) {
      // Port exists but is held by another process
      wchar_t arrDisplayName[16];
      wsprintf(arrDisplayName, L"COM%d (in use)", i);
      SendMessage(hwnd_combo, CB_ADDSTRING, 0, (LPARAM)arrDisplayName);
    }
    // ERROR_FILE_NOT_FOUND means the port simply doesn't exist (skip it)
  }

  // Auto-select the first entry if any were found
  if(SendMessage(hwnd_combo, CB_GETCOUNT, 0, 0) > 0) {
    SendMessage(hwnd_combo, CB_SETCURSEL, 0, 0);
  }
}

/// @function OpenSerialPort
/// @description Opens a COM port and configures it for Arduino communication
///              Sers baud rate, byte size, parity, stop bits, and read timeouts.
/// @param szPortName Display name e.g. L"COM3" (prefix added internally).
/// @return Valid HANDLE on success, INVALID_HANDLE_VALUE on failure.
HANDLE OpenSerialPort(const wchar_t* szPortName) {
  wchar_t arrPortPath[24];
  wsprintf(arrPortPath, L"\\\\.\\%s", szPortName);

  HANDLE hPort = CreateFile(arrPortPath,
                            GENERIC_READ | GENERIC_WRITE,
                            0, NULL, OPEN_EXISTING, 0, NULL);
  
  if(hPort == INVALID_HANDLE_VALUE) {
    return INVALID_HANDLE_VALUE;
  }

  // ────── ⋆⋅☆⋅⋆ ────────
  // DCB - Devince Control Block
  // Configure the physical serial line paramers. Must match the arduino
  // sketch's Serial.begin() call exactly or you'll recieve garbage.
  // GetCommState fills a DCB with the port's current settings so we only
  // change what we need rather than specifying every field from scratch.
  // ────── ⋆⋅☆⋅⋆ ────────
  DCB dcb_config = {};
  dcb_config.DCBlength = sizeof(DCB);

  if(!GetCommState(hPort, &dcb_config)) {
    CloseHandle(hPort);
    return INVALID_HANDLE_VALUE;
  }

  dcb_config.BaudRate = BAUD_RATE;  // Must match Serial.beign(9600)
  dcb_config.ByteSize = 8;          // 8 data bits - standard
  dcb_config.Parity = NOPARITY;     // No parity bit
  dcb_config.StopBits = ONESTOPBIT; // 1 stop bit

  if(!SetCommState(hPort, &dcb_config)) {
    CloseHandle(hPort);
    return INVALID_HANDLE_VALUE;
  }

  // ────── ⋆⋅☆⋅⋆ ────────
  // CommTimeouts
  // Controls how ReadFile behaves when bytes aren't immediately available
  // 
  // ReadIntervalTimeout         - max ms between arriving bytes before return
  // ReadTotalTimeoutMultiplier  - multiplied by requested byte count
  // ReadTotalTimeoutConstant    - added to the multiplier result
  // 
  // Setting ReadIntervalTimeout to MAXDWORD and both totals to 0 makes
  // ReadFile return immediately with whatever bytes are in the buffer,
  // rather than blocking untile the full requested count arrives.
  // This is esstional. A blocking ReadFile would freeze the message pump.
  // ────── ⋆⋅☆⋅⋆ ────────
  COMMTIMEOUTS timeouts = {};
  timeouts.ReadIntervalTimeout = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = 0;

  if(!SetCommTimeouts(hPort, &timeouts)) {
    CloseHandle(hPort);
    return INVALID_HANDLE_VALUE;
  }

  return hPort;
}

void AppendOutputText(HWND hwnd_edit, const wchar_t* szText) {
  // Move caret to end, then replace the empty selection with new text
  int iLen = GetWindowTextLength(hwnd_edit);
  SendMessage(hwnd_edit, EM_SETSEL, iLen, iLen);
  SendMessage(hwnd_edit, EM_REPLACESEL, FALSE, (LPARAM)szText);

  // Scroll to the last line
  SendMessage(hwnd_edit, EM_SCROLLCARET, 0, 0);
}

/// @function SendSerialCommand
/// @description Writes a null-terminated narrow string to an open serial port.
///              Appends a newline so Arduino's readStringUntil('\n') fires cleanly.
/// @param hPort Open port handle.
/// @param szCmd Command string e.g. "LED_ON".
/// @preturn TRUE on success, FALSE on failure.
BOOL SendSerialCommand(HANDLE hPort, const char* szCmd) {
  if(hPort == INVALID_HANDLE_VALUE) return FALSE;

  // Build "CMD\n" in a local buffer
  char arrBuf[64];
  int  iLen = wsprintfA(arrBuf, "%s\n", szCmd);

  DWORD dwBytesWritten = 0;
  return WriteFile(hPort, arrBuf, iLen, &dwBytesWritten, NULL);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  const wchar_t* szClassName = L"SerialMonitorWindow";

  WNDCLASSEX wc       = {};
  wc.cbSize           = sizeof(WNDCLASSEX);
  wc.lpfnWndProc      = WndProc;
  wc.hInstance        = hInstance;
  wc.lpszClassName    = szClassName;
  wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
  RegisterClassEx(&wc);

  HWND hwnd_main = CreateWindowEx(
    0, szClassName, L"Win32 Serial Monitor",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    610, 500, NULL, NULL, hInstance, NULL
  );

  if(hwnd_main == NULL) return -1;

  ShowWindow(hwnd_main, nCmdShow);
  UpdateWindow(hwnd_main);

  MSG msg_current = {};
  while(GetMessage(&msg_current, NULL, 0, 0)) {
    TranslateMessage(&msg_current);
    DispatchMessage(&msg_current);
  }
  return (int)msg_current.wParam;
}

/// @function WndProc
/// @description Main window message handler. Manages port lifecycle, timer polling.
///              and controll interactions.
/// @param hwnd Handle to the window.
/// @param uMsg Message ID.
/// @param wParam Message data (varies).
/// @param lParam Message data (varies).
/// @return 0 if handled, otherwise DefWindowProc
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  static HANDLE hPort            = INVALID_HANDLE_VALUE;
  static HWND   hwnd_combo_port  = NULL;
  static HWND   hwnd_btn_connect = NULL;
  static HWND   hwnd_btn_disc    = NULL;
  static HWND   hwnd_edit_output = NULL;
  static HWND   hwnd_btn_led     = NULL; 
  static bool   bLedState        = false;

  switch(uMsg) {
    // ────── ⋆⋅☆⋅⋆ ────────
    // WM_CREATE - Build all controls
    // ────── ⋆⋅☆⋅⋆ ────────
    case WM_CREATE:
    {
      // Port label
      CreateWindowEx(0, L"STATIC", L"COM Port:",
          WS_CHILD | WS_VISIBLE | SS_LEFT,
          16, 18, 80, 24, hwnd, NULL, NULL, NULL);

      // COM port dropdown
      // CBS_DROPDOWNLIST - user picks from list only, no free typeing
      hwnd_combo_port = CreateWindowEx(0, L"COMBOBOX", NULL,
          WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
          100, 14, 140, 200,
          hwnd, (HMENU)ID_COMBO_PORT, NULL, NULL);

      // Connect / Disconnect buttons
      hwnd_btn_connect = CreateWindowEx(0, L"BUTTON", L"Connect",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          256, 14, 100, 28,
          hwnd, (HMENU)ID_BTN_CONNECT, NULL, NULL);

      hwnd_btn_disc = CreateWindowEx(0, L"BUTTON", L"Disconnect",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          364, 14, 110, 28,
          hwnd, (HMENU)ID_BTN_DISCONNECT, NULL, NULL);

      // LED toggle button - disable until connected
      
      hwnd_btn_led = CreateWindowEx(0, L"Button", L"LED: OFF",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        490, 14, 100, 28,
      hwnd, (HMENU)ID_BTN_LED, NULL, NULL);

      EnableWindow(hwnd_btn_led, FALSE);
      
      // Output display  - multiline, read-only, scrollable EDIT control
      // ES_MULTILINE    - Multiple lines
      // ES_READONLY     - user can't type into it
      // ES_AUTOVSCROLL  - scrolls vertically as content grows
      // WS_VSCROLL      - shows a vertical scrollbar
      hwnd_edit_output = CreateWindowEx(
          WS_EX_CLIENTEDGE, // Sunken border - conventional for output boxes
          L"EDIT", NULL,
          WS_CHILD | WS_VISIBLE | WS_VSCROLL |
          ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
          16, 56, 552, 380,
          hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);

      // Disable Disconnect until we're actually connected
      EnableWindow(hwnd_btn_disc, FALSE);

      // Scan and populate the port dropdown
      ScanComPorts(hwnd_combo_port);

      return 0;
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // MW_COMMAND
    // ────── ⋆⋅☆⋅⋆ ────────
    case WM_COMMAND:
    {
      switch(LOWORD(wParam)) {
        case ID_BTN_CONNECT:
        {
          // Read selected port name from dropdown.
          // CB_GETCOUNT guard avoids calling GetWindowText on an
          // empty dropdown.
          if(SendMessage(hwnd_combo_port, CB_GETCOUNT, 0, 0) == 0) {
            MessageBox(hwnd, L"No COM ports found.",
                       L"Connect", MB_OK | MB_ICONWARNING);
            return 0;
          }

          wchar_t arrPortName[16];
          GetWindowText(hwnd_combo_port, arrPortName, 16);

          // Strip " (in use)" suffix if present before opening
          // wcschr finds the first space so "COM 3 (in use)" -> "COM3"
          wchar_t* pSpace = wcschr(arrPortName, L' ');
          if(pSpace) *pSpace = L'\0';

          hPort = OpenSerialPort(arrPortName);

          if(hPort == INVALID_HANDLE_VALUE) {
            wchar_t arrMsg[64];
            wsprintf(arrMsg, L"Failed to open %s. \nError:%lu",
                    arrPortName, GetLastError());
            MessageBox(hwnd, arrMsg, L"Connent",
                       MB_OK | MB_ICONERROR);
            return 0;
          }

          // Connected - start polling timer, swap button states
          SetTimer(hwnd, ID_TIMER_POLL, POLL_MS, NULL);
          EnableWindow(hwnd_btn_connect, FALSE);
          EnableWindow(hwnd_btn_disc,    TRUE);
          EnableWindow(hwnd_combo_port,  FALSE);
          EnableWindow(hwnd_btn_led,     TRUE);

          wchar_t arrStatus[32];
          wsprintf(arrStatus, L"[Connected to %s]\r\n", arrPortName);
          AppendOutputText(hwnd_edit_output, arrStatus);
          return 0;
        }

        case ID_BTN_DISCONNECT:
        {
          KillTimer(hwnd, ID_TIMER_POLL);

          if(hPort != INVALID_HANDLE_VALUE) {
            CloseHandle(hPort);
            hPort = INVALID_HANDLE_VALUE;
          }

          EnableWindow(hwnd_btn_connect, TRUE);
          EnableWindow(hwnd_btn_disc,    FALSE);
          EnableWindow(hwnd_combo_port,  TRUE);
          EnableWindow(hwnd_btn_led,     FALSE);
          SetWindowText(hwnd_btn_led, L"LED: OFF");
          bLedState = false;

          AppendOutputText(hwnd_edit_output, L"[Disconnected]\r\n");

          // Rescan ports in case something changed
          ScanComPorts(hwnd_combo_port);
          return 0;
        }

        case ID_BTN_LED:
        {
          bLedState = !bLedState; // Toggle tracked state

          if(bLedState) {
            SendSerialCommand(hPort, "LED_ON");
            SetWindowText(hwnd_btn_led, L"LED: ON");
          } else {
            SendSerialCommand(hPort, "LED_OFF");
            SetWindowText(hwnd_btn_led, L"LED: OFF");
          }
          return 0;
        }
      }
      return 0;
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // WM_TIMER - Poll the serial port for incoming bytes
    // ReadFile with our COMMTIMEOUTS config returns immediately with
    // however many bytes are waiting, or 0 bytes if the buffer is empty
    // ────── ⋆⋅☆⋅⋆ ────────
    case WM_TIMER:
    {
      if(wParam != ID_TIMER_POLL) break;
      if(hPort == INVALID_HANDLE_VALUE) break;

      char arrRawBuf[READ_BUF];
      DWORD dwBytesRead = 0;

      ReadFile(hPort, arrRawBuf, READ_BUF -1, &dwBytesRead, NULL);

      if(dwBytesRead > 0) {
        // ReadFile gives us narrow chars - convert to wide for Win32
        arrRawBuf[dwBytesRead] = '\0';

        wchar_t arrWideBuf[READ_BUF];
        MultiByteToWideChar(CP_ACP, 0,
                            arrRawBuf, -1,
                            arrWideBuf, READ_BUF);

        AppendOutputText(hwnd_edit_output, arrWideBuf);
      }
      return 0;
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // WM_DESTROY
    // ────── ⋆⋅☆⋅⋆ ────────
    case WM_DESTROY:
    {
      KillTimer(hwnd, ID_TIMER_POLL);
      if(hPort != INVALID_HANDLE_VALUE) {
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;
      }

      PostQuitMessage(0);
      return 0;
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}