/// @file OhmsLaw.cpp
/// @brief Ohm's Law Visualizer
///        Parses live V/I/R/P data from the Arduino
///        and displays it as labeled numeric readouts

#include <windows.h>
#include <stdlib.h>
#include "..\pch.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_COMBO_PORT    101
#define ID_BTN_CONNECT   102
#define ID_BTN_DISCONNECT 103
#define ID_TIMER_POLL    1

#define POLL_MS  100
#define READ_BUF 256

// ────── ⋆⋅☆⋅⋆ ────────
// Scan COM ports
// ────── ⋆⋅☆⋅⋆ ────────

/// @function ScanComPorts
/// @brief Probes COM1-COM20 and populates a ComboBox with
///        available ports
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

    if(hTest != INVALID_HANDLE_VALUE) {
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
// Readout Panel struct
// A label + value field pair
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Groups a static label and a read-only value display field
struct ReadoutPanel {
  Label* lbl_name  = nullptr;
  Label* lbl_value = nullptr;
  Label* lbl_unit  = nullptr;

  /// @function Create
  /// @brief Instantiates the label, value, and unit fields
  /// @param hwnd_parent Parent window
  /// @param szName Descriptor e.g. L"Voltage"
  /// @param szUnit Unit string e.g. L"V"
  /// @param iX Left position
  /// @param iY Top position
  void Create(HWND hwnd_parent, const wchar_t* szName, const wchar_t* szUnit, int iX, int iY) {
    // Row height 28px, name col 90px, value col 100px, unit col 40px
    lbl_name  = new Label(hwnd_parent, szName, iX,       iY,  90, 24);
    lbl_value = new Label(hwnd_parent, L"---", iX + 95,  iY, 100, 24);
    lbl_unit  = new Label(hwnd_parent, szUnit, iX + 200, iY,  40, 24);
  }

  /// @function SetValue
  /// @brief Updates the displayed value string.
  /// @param szValue Formatted number string
  void SetValue(const wchar_t* szValue) {
    if(lbl_value) lbl_value->SetText(szValue);
  }

  /// @function Destroy
  /// @brief Frees heap-allocated labels.
  void Destroy() {
    delete lbl_name;
    delete lbl_value;
    delete lbl_unit;
    lbl_name = lbl_value = lbl_unit = nullptr;
  }
};

// ────── ⋆⋅☆⋅⋆ ────────
// Ohm's Law Window
// ────── ⋆⋅☆⋅⋆ ────────

class OhmsLawWindow : public Window {

  public:
    OhmsLawWindow()
      : cmb_port(nullptr)
      , btn_connect(nullptr)
      , btn_disc(nullptr)
    {}

    ~OhmsLawWindow() {
      delete cmb_port;
      delete btn_connect;
      delete btn_disc;

      panel_v.Destroy();
      panel_i.Destroy();
      panel_r.Destroy();
      panel_p.Destroy();
    }

  protected:
    // ────── ⋆⋅☆⋅⋆ ────────
    //      OnCreate
    //   Build controls
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnCreate() override {
      // ────── ⋆⋅☆⋅⋆ ────────
      // ---Connection bar---
      // ────── ⋆⋅☆⋅⋆ ────────
      new Label(hwnd_self, L"COM PORT:", 16, 18, 80, 24);

      cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 100, 14, 140,200);
      btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, 256, 14, 100, 28);
      btn_disc = new Button (hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 364, 14, 110, 28);

      btn_disc->Disable();
      ScanComPorts(cmb_port);

      // ────── ⋆⋅☆⋅⋆ ────────
      // Divider label
      // ────── ⋆⋅☆⋅⋆ ────────
      new Label(hwnd_self, L"────────────────────────────────⋆⋅☆⋅⋆────────────────────────────────", 16, 54, 400, 20);

      // ────── ⋆⋅☆⋅⋆ ────────
      // Readout panels
      // ────── ⋆⋅☆⋅⋆ ────────
      panel_v.Create(hwnd_self, L"Voltage",    L"V",  30,  86);
      panel_i.Create(hwnd_self, L"Current",    L"mA", 30, 126);
      panel_r.Create(hwnd_self, L"Resistance", L"Ω",  30, 166);
      panel_p.Create(hwnd_self, L"Power",      L"mW", 30, 206);

      // ────── ⋆⋅☆⋅⋆ ────────
      // R_known reminder label
      // ────── ⋆⋅☆⋅⋆ ────────
      new Label(hwnd_self, L"R_known = 1000Ω  |  V_ref = 5.0V", 16, 256, 300, 20);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnCommand: connect/disconnect
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnCommand(int iControlId, int iNotifCode) override {
      switch(iControlId) {
        case ID_BTN_CONNECT: OnConnect(); break;
        case ID_BTN_DISCONNECT: OnDisconnect(); break;
      }
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnTimer:
    // poll serial, parse message, update panels
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnTimer(int iTimerId) override {
      if(iTimerId != ID_TIMER_POLL) return;
      if(!port.IsOpen()) return;

      char arrRawBuf[READ_BUF];
      DWORD dwRead = 0;
      port.Read(arrRawBuf, READ_BUF, dwRead);

      if(dwRead == 0) return;
      arrRawBuf[dwRead] = '\0';

      // Convert to wide string for parsing
      wchar_t arrWideBuf[READ_BUF];
      MultiByteToWideChar(CP_ACP, 0, arrRawBuf, -1, arrWideBuf, READ_BUF);

      ParseAndDisplay(arrWideBuf);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnDestroy
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnDestroy() override {
      KillTimer(hwnd_self, ID_TIMER_POLL);
      port.Close();
    }

  private:
    // ────── ⋆⋅☆⋅⋆ ────────
    // Parse and Display
    // Expects format: "V:x.xxx,I:x.xxxx,R:xxxx.x,P:x.xxxxx\n"
    // Strategy: find each key, step past the colon, read the float value
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function ParseFloat
    /// @brief Finds szKey in szSrc, steps past the colon, and
    ///        returns the float value that folloes. Returns
    ///        -1.0 if the key isn't found.
    /// @param szSrc The full incoming message string
    /// @param szKey Key to search for
    /// @return Parsed float, or -1.0 on failure
    float ParseFloat(const wchar_t* szSrc, const wchar_t* szKey) {
      const wchar_t* pFound = wcsstr(szSrc, szKey);
      if(!pFound) return -1.0f;

      // Advance past the key
      pFound += wcslen(szKey);
      return wcstof(pFound, nullptr);
    }

    /// @function ParseAndDisplay
    /// @brief Parses a full serial message and updates all
    ///        four readout panels. Skips lines that don't
    ///        contain the expected keys (e.g. the startup message)
    /// @param szMsg Wide string containin the incoming serial line.
    /// @retrn void
    void ParseAndDisplay(const wchar_t* szMsg) {
      float fV = ParseFloat(szMsg, L"V:");
      float fI = ParseFloat(szMsg, L"I:");
      float fR = ParseFloat(szMsg, L"R:");
      float fP = ParseFloat(szMsg, L"P:");

      // Any negative resulr means the key wasn't found. Skip
      if(fV < 0 || fI < 0 || fR < 0 || fP < 0) return;

      // Format each value for display
      // Current and power are scaled to mA and mW for readability
      wchar_t arrBuf[32];

      // Volts in V
      wsprintf(arrBuf, L"%d.%03d", (int)fV, (int)((fV - (int)fV) * 1000));
      panel_v.SetValue(arrBuf);

      // Current in mA
      float fI_mA = fI * 1000.0f;
      wsprintf(arrBuf, L"%d.%03d", (int)fI_mA, (int)((fI_mA - (int)fI_mA) * 1000));
      panel_i.SetValue(arrBuf);

      // Resistance in ohms
      wsprintf(arrBuf, L"%d", (int)fR);
      panel_r.SetValue(arrBuf);

      // Power in mW
      float fP_mW = fP * 1000.0f;
      wsprintf(arrBuf, L"%d.%02d", (int)fP_mW, (int)((fP_mW - (int)fP_mW) * 100));
      panel_p.SetValue(arrBuf);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // On Connect/Disconnect
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnConnect() {
      if(cmb_port->GetCount() == 0) {
        MessageBox(hwnd_self, L"No COM ports found.", L"Connect", MB_OK | MB_ICONWARNING);
        return;
      }

      wchar_t arrPortName[16];
      cmb_port->GetSelected(arrPortName, 16);

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
      cmb_port->Disable();
    }

    void OnDisconnect() {
      KillTimer(hwnd_self, ID_TIMER_POLL);
      port.Close();

      btn_connect->Enable();
      btn_disc->Disable();
      cmb_port->Enable();

      panel_v.SetValue(L"---");
      panel_i.SetValue(L"---");
      panel_r.SetValue(L"---");
      panel_p.SetValue(L"---");

      ScanComPorts(cmb_port);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Members
    // ────── ⋆⋅☆⋅⋆ ────────
    SerialPort port;

    ComboBox* cmb_port;
    Button* btn_connect;
    Button* btn_disc;

    ReadoutPanel panel_v;
    ReadoutPanel panel_i;
    ReadoutPanel panel_r;
    ReadoutPanel panel_p;
};

// ────── ⋆⋅☆⋅⋆ ────────
// WinMain
// ────── ⋆⋅☆⋅⋆ ────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  OhmsLawWindow win_main;
  win_main.Create(hInstance, L"Ohm's Law Visualizer", 520, 330);
  win_main.Show(nCmdShow);
  return win_main.Run();
}