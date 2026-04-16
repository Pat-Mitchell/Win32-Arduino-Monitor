/// @file win32_serial3.cpp
/// @brief Serial monitor rebuilt using SerialPort wrapper

#include <windows.h>
#include "..\wrappers\win32Wrappers\Window.h"
#include "..\wrappers\win32Wrappers\Button.h"
#include "..\wrappers\win32Wrappers\Label.h"
#include "..\wrappers\win32Wrappers\TextInput.h"
#include "..\wrappers\win32Wrappers\ComboBox.h"
#include "..\wrappers\serialPortWrappers\SerialPort.h"

#define ID_COMBO_PORT     101
#define ID_BTN_CONNECT    102
#define ID_BTN_DISCONNECT 103
#define ID_EDIT_OUTPUT    104
#define ID_BTN_LED        105
#define ID_TIMER_POLL     1

#define POLL_MS 100
#define READ_BUF 256

/// @function ScanComPorts
/// @brief Probes COM1-COM20 and populates a ComboBox with available ports
/// @param cmb_port Target ComboBox
/// @return void
void ScanComPorts(ComboBox* cmb_port) {
  cmb_port->Clear();

  for(int i = 1; i <=20; i++) {
    wchar_t arrPortPath[16];
    wsprintf(arrPortPath, L"\\\\.\\CO%d", i);

    HANDLE hTest = CreateFile(arrPortPath,
                              GENERIC_READ | GENERIC_WRITE,
                              0, NULL, OPEN_EXISTING, 0, NULL);

    if(hTest != INVALID_HANDLE_VALUE) {
      wchar_t arrName[8];
      wsprintf(arrName, L"COM%d", i);
      cmb_port->AddItem(arrName);
      CloseHandle(hTest);
    }else if(GetLastError() == ERROR_ACCESS_DENIED) {
      wchar_t arrName[16];
      wsprintf(arrName, L"COM%d (in use)", i);
      cmb_port->AddItem(arrName);
    }
  }

  cmb_port->SelectFirst();
}

class SerialMonitorWindow : public Window {
  public:
    SerialMonitorWindow()
      : bLedState(false)
      , cmb_port(nullptr)
      , btn_connect(nullptr)
      , btn_disc(nullptr)
      , btn_led(nullptr)
      , txt_output(nullptr)
    {}

    ~SerialMonitorWindow() {
      delete cmb_port;
      delete btn_connect;
      delete btn_disc;
      delete btn_led;
      delete txt_output;
    }
  
  protected:
    void OnCreate() override {
      new Label(hwnd_self, L"COM Port:", 16, 18, 80, 24);

      cmb_port    = new ComboBox  (hwnd_self, ID_COMBO_PORT, 100, 14, 140, 200);
      btn_connect = new Button    (hwnd_self, L"Connect", ID_BTN_CONNECT, 256, 14, 100, 28);
      btn_disc    = new Button    (hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 364, 14, 110, 28);
      btn_led     = new Button    (hwnd_self, L"LED: OFF", ID_BTN_LED, 490, 14, 100, 28);
      txt_output  = new TextInput (hwnd_self, ID_EDIT_OUTPUT, 16, 56, 552, 380, TRUE);

      txt_output->SetReadOnly(TRUE);
      btn_disc->Disable();
      btn_led->Disable();

      ScanComPorts(cmb_port);
    }

    void OnCommand(int iControlId, int iNotifCode) override {
      switch (iControlId) {
        case ID_BTN_CONNECT:    OnConnect();    break;
        case ID_BTN_DISCONNECT: OnDisconnect(); break;
        case ID_BTN_LED:        OnToggleLed();  break;
      }
    }

    void OnTimer(int iTimerId) override {
      if(iTimerId != ID_TIMER_POLL) return;
      if(!port.IsOpen()) return;

      char arrRawBuf[READ_BUF];
      DWORD dwRead = 0;

      // Read returns immediately. 0 bytes is normal when buffer is empty
      port.Read(arrRawBuf, READ_BUF, dwRead);

      if(dwRead > 0) {
        arrRawBuf[dwRead] = '\0';

        wchar_t arrWideBuf[READ_BUF];
        MultiByteToWideChar(CP_ACP, 0, arrRawBuf, -1, arrWideBuf, READ_BUF);
        txt_output->Append(arrWideBuf);
      }
    }

    void OnDestroy() override {
      KillTimer(hwnd_self, ID_TIMER_POLL);
      port.Close(); // SerialPort destructor also calls this
    }
  private:
    void OnConnect() {
      if(cmb_port->GetCount() == 0) {
        MessageBox(hwnd_self, L"No COM ports found.", L"Connect", MB_OK | MB_ICONWARNING);
        return;
      }

      wchar_t arrPortName[16];
      cmb_port->GetSelected(arrPortName, 16);

      // SerialPort::Open strips the "(in use)" suffix internally
      if(!port.Open(arrPortName)) {
        wchar_t arrMsg[64];
        wsprintf(arrMsg, L"Failed to open %s.\nError: %lu",
                arrPortName, port.GetLastErrorCode());
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
      port.Close();

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
        port.Write("LED_ON");
        btn_led->SetText(L"LED: ON");
      } else {
        port.Write("LED_OFF");
        btn_led->SetText(L"LED: OFF");
      }
    }

    SerialPort port;
    bool       bLedState;

    ComboBox*  cmb_port;
    Button*    btn_connect;
    Button*    btn_disc;
    Button*    btn_led;
    TextInput* txt_output;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  SerialMonitorWindow win_main;
  win_main.Create(hInstance, L"Serial Monitor", 620, 500);
  win_main.Show(nCmdShow);
  return win_main.Run();
}