/// @file win32_serial2.cpp
/// @brief Serial monitor app rebuilt using the Win32 wrapper classes.
///        Compare the size of WinMain and OnCreate here vs win32_serial.cpp.

#include <windows.h>
#include "..\wrappers\win32Wrappers\Window.h"
#include "..\wrappers\win32Wrappers\Button.h"
#include "..\wrappers\win32Wrappers\Label.h"
#include "..\wrappers\win32Wrappers\TextInput.h"
#include "..\wrappers\win32Wrappers\ComboBox.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Controls IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_COMBO_PORT     101
#define ID_BTN_CONNECT    102
#define ID_BTN_DISCONNECT 103
#define ID_EDIT_OUTPUT    104
#define ID_BTN_LED        105
#define ID_TIMER_POLL     1

#define BAUD_RATE  9600
#define POLL_MS    100
#define READ_BUF   256

/// @function OpenSerialPort
/// @brief Opens and configures a COM port for Arduino Communication
/// @param szPortName Display name
/// @return Valid HANDLE on success. INVAILD_HANDLE_VALUE on failure
HANDLE OpenSerialPort(const wchar_t* szPortName) {
  wchar_t(arrPortPath[24]);
  wsprintf(arrPortPath, L"\\\\.\\%s", szPortName);

  HANDLE hPort = CreateFile(arrPortPath,
                            GENERIC_READ | GENERIC_WRITE,
                            0, NULL, OPEN_EXISTING, 0, NULL);
  
  if(hPort == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;

  DCB dcb_config       = {};
  dcb_config.DCBlength = sizeof(DCB);

  if(!GetCommState(hPort, &dcb_config)) {
    CloseHandle(hPort); 
    return INVALID_HANDLE_VALUE;
  }

  dcb_config.BaudRate = BAUD_RATE;
  dcb_config.ByteSize = 8;
  dcb_config.Parity   = NOPARITY;
  dcb_config.StopBits = ONESTOPBIT;

  if(!SetCommState(hPort, &dcb_config)) {
    CloseHandle(hPort);
    return INVALID_HANDLE_VALUE;
  }

  COMMTIMEOUTS timeouts               = {};
  timeouts.ReadIntervalTimeout        = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant   = 0;

  if(!SetCommTimeouts(hPort, &timeouts)) {
    CloseHandle(hPort);
    return INVALID_HANDLE_VALUE;
  }

  return hPort;
}

/// @function SendSerialCommand
/// @brief Writes a command string + newline to an open serial port.
/// @param hPort Open port handle.
/// @param szCmd Narrow command string
/// @return TRUE on success.
BOOL SendSerialCommand(HANDLE hPort, const char* szCmd) {
  if(hPort = INVALID_HANDLE_VALUE) return FALSE;

  char arrBuf[64];
  int iLen = wsprintfA(arrBuf, "%s\n", szCmd);

  DWORD dwWritten = 0;
  return WriteFile(hPort, arrBuf, iLen, &dwWritten, NULL);
}

/// @function ScanComPorts
/// @brief Probes COM1-COM20 and populates a ComboBox with available ports.
/// @param cmb_port Target ComboBox
/// @return void
void ScanComPorts(ComboBox* cmb_port) {
  cmb_port->Clear();

  for(int i = 1; i <= 20; i++) {
    wchar_t arrPortPath[16];
    wsprintf(arrPortPath, L"\\\\.\\COM%d", i);

    HANDLE hTest = CreateFile(arrPortPath,
                              GENERIC_READ | GENERIC_WRITE,
                              0, NULL, OPEN_EXISTING, 0, NULL);
                            
    if(hTest!= INVALID_HANDLE_VALUE) {
      wchar_t arrName[8];
      wsprintf(arrName, L"COM%d", i);
      cmb_port->AddItem(arrName);
      CloseHandle(hTest);
    } else if(GetLastError() == ERROR_ACCESS_DENIED) {
      wchar_t arrName[16];
      wsprintf(arrName, L"COM%d (in use)", i);
      cmb_port->AddItem(arrName);
    }
  }

  cmb_port->SelectFirst();
}

// ────── ⋆⋅☆⋅⋆ ────────
// SerialMonitorWindow
// the application window
// ────── ⋆⋅☆⋅⋆ ────────

class SerialMonitorWindow : public Window {
  public:

    SerialMonitorWindow()
      : hPort(INVALID_HANDLE_VALUE)
      , bLedState(false)
      , cmb_port(nullptr)
      , btn_connect(nullptr)
      , btn_disc(nullptr)
      , btn_led(nullptr)
      , txt_output(nullptr)
    {}

    ~SerialMonitorWindow() {
      // Heap-allocated controls freed here
      delete cmb_port;
      delete btn_connect;
      delete btn_disc;
      delete btn_led;
      delete txt_output;
    }

  protected:
    /// @function OnCreate
    /// @brief Builds all child controls and performs initial port scan
    void OnCreate() override {
      Label* lbl_port = new Label(hwnd_self, L"COM port:", 16, 18, 80, 24);
      // lbl_port is decorative
      (void)lbl_port;

      cmb_port    = new ComboBox (hwnd_self, ID_COMBO_PORT, 100, 14, 140, 200);
      btn_connect = new Button (hwnd_self, L"Connect", ID_BTN_CONNECT, 256, 14, 100, 28);
      btn_disc    = new Button (hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 364, 14, 110, 28);
      btn_led     = new Button (hwnd_self, L"LED: OFF", ID_BTN_LED, 490, 14, 100, 28);
      txt_output  = new TextInput (hwnd_self, ID_EDIT_OUTPUT, 16, 56, 552, 380, TRUE);

      txt_output->SetReadOnly(TRUE);
      btn_disc->Disable();
      btn_led->Disable();

      ScanComPorts(cmb_port);
    }

    /// @function OnCommand
    /// @brief Handles button clicks via control ID
    /// @param iControlId The ID of the control that fired
    /// @param iNotifCode Notification code
    void OnCommand(int iControlId, int iNotifCode) override {
      switch(iControlId) {
        case ID_BTN_CONNECT:    OnConnect();    break;
        case ID_BTN_DISCONNECT: OnDisconnect(); break;
        case ID_BTN_LED:        OnToggleLed();  break;
      }
    }

    /// @function OnTimer
    /// @brief Polls the serial port for incoming bytes
    /// @param iTimerId  Timer ID
    void OnTimer(int iTimerId) override {

      if(iTimerId != ID_TIMER_POLL) return;
      if(hPort == INVALID_HANDLE_VALUE) return;

      char arrRawBuf[READ_BUF];
      DWORD dwBytesRead = 0;

      ReadFile(hPort, arrRawBuf, READ_BUF - 1, &dwBytesRead, NULL);

      if(dwBytesRead > 0) {
        arrRawBuf[dwBytesRead] = '\0';

        wchar_t arrWideBuf[READ_BUF];
        MultiByteToWideChar(CP_ACP, 0, arrRawBuf, -1, arrWideBuf, READ_BUF);
        txt_output->Append(arrWideBuf);
      }
    }

    /// @function OnDestroy
    /// @brief Ensures port and timer are released on close
    void OnDestroy() override {
      KillTimer(hwnd_self, ID_TIMER_POLL);

      if(hPort != INVALID_HANDLE_VALUE) {
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;
      }
    }

  private:

    // ────── ⋆⋅☆⋅⋆ ────────
    // Privare action methods
    // ────── ⋆⋅☆⋅⋆ ────────

    void OnConnect() {
      if(cmb_port->GetCount() == 0) {
        MessageBox(hwnd_self, L"No COM ports found.",
                   L"Connect", MB_OK | MB_ICONWARNING);
        return;
      }

      wchar_t arrPortName[16];
      cmb_port->GetSelected(arrPortName, 16);

      // Strip " (in use)" suffix if present
      wchar_t* pSpace = wcschr(arrPortName, L' ');
      if(pSpace) *pSpace = L'\0';

      hPort = OpenSerialPort(arrPortName);

      if(hPort == INVALID_HANDLE_VALUE) {
        wchar_t arrMsg[64];
        wsprintf(arrMsg, L"Failed to open %s.\nError: %lu",
                 arrPortName, GetLastError());
        MessageBox(hwnd_self, arrMsg, L"Connect", MB_OK | MB_ICONERROR);
        return;
      }

      SetTimer(hwnd_self, ID_TIMER_POLL, POLL_MS, NULL);

      btn_connect->Disable();
      btn_disc->Enable();
      btn_led->Enable();
      cmb_port->Disable();

      wchar_t arrStatus[32];
      wsprintf(arrStatus, L"[Connected to %s]\r\n", arrPortName);
      txt_output->Append(arrStatus);
    }

    void OnDisconnect() {
      KillTimer(hwnd_self, ID_TIMER_POLL);

      if(hPort != INVALID_HANDLE_VALUE) {
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;
      }

      btn_connect->Enable();
      btn_disc->Disable();
      btn_led->Disable();
      btn_led->SetText(L"LED: OFF");
      cmb_port->Enable();

      bLedState = false;

      txt_output->Append(L"[Disconnected]\r\n");
      ScanComPorts(cmb_port);
    }

    void OnToggleLed() {
      bLedState = !bLedState;

      if(bLedState) {
        SendSerialCommand(hPort, "LED_ON");
        btn_led->SetText(L"LED: ON");
      } else {
        SendSerialCommand(hPort, "LED_OFF");
        btn_led->SetText(L"LED: OFF");
      }
    }


    // ────── ⋆⋅☆⋅⋆ ────────
    // Member variables
    // ────── ⋆⋅☆⋅⋆ ────────
    HANDLE     hPort;
    bool       bLedState;
    
    ComboBox*  cmb_port;
    Button*    btn_connect;
    Button*    btn_disc;
    Button*    btn_led;
    TextInput* txt_output;
};

// ────── ⋆⋅☆⋅⋆ ────────
// WinMain
// ────── ⋆⋅☆⋅⋆ ────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  SerialMonitorWindow win_main;
  win_main.Create(hInstance, L"Serial Monitor", 620, 500);
  win_main.Show(nCmdShow);
  return win_main.Run();
}