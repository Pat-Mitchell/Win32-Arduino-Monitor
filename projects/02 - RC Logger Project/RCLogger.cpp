/// @file RCLogger.cpp
/// @brief RC Circuit Time Constant Logger
///        Sends CHARGEG/DISCHARGE Commands to the arduino, plots the
///        incoming voltage curve live, and caluculates tau from the data

#include <windows.h>
#include <stdlib.h>
#include <cstdio>
#include "..\pch.h"
#include "..\Utils\Utils.h"

// ─────── ⋆⋅☆⋅⋆ ────────
//     Control IDS
// ─────── ⋆⋅☆⋅⋆ ────────
#define ID_COMBO_PORT     101
#define ID_BTN_CONNECT    102
#define ID_BTN_DISCONNECT 103
#define ID_BTN_CHARGE     104
#define ID_BTN_DISCHARGE  105
#define ID_BTN_CLEAR      106
#define ID_BTN_EXPORT     107
#define ID_TIMER_POLL       1

#define POLL_MS            50
#define READ_BUF          512
#define MAX_SAMPLES      1024

// ─────── ⋆⋅☆⋅⋆ ────────
// Known component values
// ─────── ⋆⋅☆⋅⋆ ────────
#define R_KNOWN 10000.0f // Ohms
#define C_KNOWN 0.0001f // Farads
#define TAU_THEORY (R_KNOWN * C_KNOWN)

// ─────── ⋆⋅☆⋅⋆ ────────
//      Plot Panel
// ─────── ⋆⋅☆⋅⋆ ────────
// Owns the time/voltage dataset and render it into a caller-supplied HDC.
// The bounding rect is set once at creation and refereced every paint.

struct PlotPanel {
  // Dataset
  float arrTime[MAX_SAMPLES];
  float arrVolt[MAX_SAMPLES];
  int iCount;
  int iPadL, iPadT, iPadR, iPadB;

  // Bounding rect in client coordinates
  RECT rect_bounds;

  /// @param iX Left edge of the plot area
  /// @param iY Top edge
  /// @param iW Width in pixels
  /// @param iH Height in pixels
  /// @brief Sotres the bounding rect and resets the dataset
  void Init(int iX, int iY, int iW, int iH) {
    rect_bounds = { iX, iY, iX + iW, iY + iH };
    iPadL = 50; iPadT = 16; iPadR = 16; iPadB = 36;
    Clear();
  }

  /// @param fT Elepased time in milliseconds
  /// @param fV Voltage in volts
  /// @brief Appends a time/voltage pair to the dataset
  void AddSample(float fT, float fV) {
    if(iCount >= MAX_SAMPLES) return;
    arrTime[iCount] = fT;
    arrVolt[iCount] = fV;
    iCount++;
  }

  /// @brief Resets the dataset to empty
  void Clear() {
    iCount = 0;
    ZeroMemory(arrTime, sizeof(arrTime));
    ZeroMemory(arrVolt, sizeof(arrVolt));
  }

  /// @param fT Time in milliseconds
  /// @param fTmax Maximum time value in the dataset
  /// @returns Pixel X within the plot area
  /// @brief Maps an elapsed time value to a pixel X coordinate
  int MapX(float fT, float fTmax) const {
    int iPlotW = (rect_bounds.right - rect_bounds.left) - iPadL - iPadR;
    return rect_bounds.left + iPadL + (int)((fT / fTmax) * iPlotW);
  }

  /// @param fV Voltage in volts
  /// @returns PixelY within the plot area
  /// @brief Maps a voltage value (0-5V) to a pixel Y coordinate
  ///        Y is inverted so 0V maps to bottom and 5V maps to top
  int MapY(float fV) const {
    int iPlotH = (rect_bounds.bottom - rect_bounds.top) - iPadT - iPadB;
    return rect_bounds.top + iPadT + iPlotH - (int)((fV / 5.0f) * iPlotH);
  }

  // ─────── ⋆⋅☆⋅⋆ ────────
  //         Draw
  // ─────── ⋆⋅☆⋅⋆ ────────

  /// @param hdc Device context to draw into
  /// @brief Renders the plot background, axes, tau marker, and data curve.
  void Draw(HDC hdc) const {
    int iL = rect_bounds.left;
    int iT = rect_bounds.top;
    int iR = rect_bounds.right;
    int iB = rect_bounds.bottom;

    // ─────── ⋆⋅☆⋅⋆ ────────
    //       Background
    // ─────── ⋆⋅☆⋅⋆ ────────
    HBRUSH hbr_bg = CreateSolidBrush(RGB(20,20,20));
    RECT rect_fill = rect_bounds;
    FillRect(hdc, &rect_fill, hbr_bg);
    DeleteObject(hbr_bg);

    // ─────── ⋆⋅☆⋅⋆ ────────
    //         Axes
    // ─────── ⋆⋅☆⋅⋆ ────────
    HPEN hpen_axis = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    SelectObject(hdc, hpen_axis);

    // Y axis
    MoveToEx(hdc, iL + iPadL, iT + iPadT, NULL);
    LineTo (hdc, iL + iPadL, iB - iPadB);

    // X axis 
    MoveToEx(hdc, iL + iPadL, iB - iPadB, NULL);
    LineTo (hdc, iR - iPadR, iB - iPadB);

    DeleteObject(hpen_axis);

    // Axis labels
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(140, 140, 140));

    TextOut(hdc, iL + iPadL - 36, MapY(5.0f) - 6, L"5V", 2);
    TextOut(hdc, iL + iPadL - 36, MapY(2.5f) - 6, L"2.5V", 3);
    TextOut(hdc, iL + iPadL - 36, MapY(0.0f) - 6, L"0V", 2);

    TextOut(hdc, iL + iPadL - 4, iB - iPadB + 6, L"0", 1);

    // ─────── ⋆⋅☆⋅⋆ ────────
    // 63.2% dashed tau marker line
    // ─────── ⋆⋅☆⋅⋆ ────────
    HPEN hpen_tau = CreatePen(PS_DASH, 1, RGB(80, 160, 80));
    SelectObject(hdc, hpen_tau);

    int iTauY = MapY(5.0f * 0.632f); // 63.2% of 5V = 3.16V
    MoveToEx(hdc, iL + iPadL, iTauY, NULL);
    LineTo (hdc, iR - iPadR, iTauY);

    DeleteObject(hpen_tau);

    SetTextColor(hdc, RGB(80, 160, 80));
    TextOut(hdc, iL + iPadL + 4, iTauY - 14, L"63.2% (τ)", 9);

    // ─────── ⋆⋅☆⋅⋆ ────────
    //      Data curve
    // ─────── ⋆⋅☆⋅⋆ ────────
    if(iCount < 2) return;

    float fTmax = arrTime[iCount - 1];
    if(fTmax < 1.0f) fTmax = 1.0f;

    HPEN hpen_curve = CreatePen(PS_SOLID, 2, RGB(80, 180, 255));
    SelectObject(hdc, hpen_curve);

    MoveToEx(hdc, MapX(arrTime[0], fTmax), MapY(arrVolt[0]), NULL);
    for(int i = 1; i < iCount; i++) {
      LineTo(hdc, MapX(arrTime[i], fTmax), MapY(arrVolt[i]));
    }

    DeleteObject(hpen_curve);

    // x axis time label at end of curve
    wchar_t arrTmax[16];
    wsprintf(arrTmax, L"%dms", (int)fTmax);
    SetTextColor(hdc, RGB(140, 140, 140));
    TextOut(hdc, iR - iPadR - 28, iB - iPadB + 6, arrTmax, lstrlen(arrTmax));
  }
};

// ─────── ⋆⋅☆⋅⋆ ────────
// ExportCSV - app level, RC logger colums only
// ─────── ⋆⋅☆⋅⋆ ────────

/// @param szPath Full path from save dialog
/// @param arrTime Elapsed time array in milliseconds
/// @param arrVolt Voltage array in volts
/// @param iCount Number of samples
/// @returns TRUE on success.
/// @brief Write the time/voltage dataset to a CSV file
BOOL ExportCSV(const wchar_t* szPath, const float* arrTime, const float* arrVolt, int iCount) {
  HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hFile == INVALID_HANDLE_VALUE) return FALSE;

  DWORD dwWritten = 0;
  const char* szHeader = "Time_ms,Voltage_V\r\n";
  WriteFile(hFile, szHeader, lstrlenA(szHeader), &dwWritten, NULL);

  for(int i = 0; i < iCount; i++) {
    char arrRow[64];
    int iWhole = (int)arrVolt[i];
    int iFrac = (int)((arrVolt[i] - iWhole) * 1000);
    int iLen = wsprintfA(arrRow, "%d,%d.%03d\r\n", (int)arrTime[i], iWhole, iFrac);
    WriteFile(hFile, arrRow, iLen, &dwWritten, NULL);
  }

  CloseHandle(hFile);
  return TRUE;
}

// ─────── ⋆⋅☆⋅⋆ ────────
// === RCLoggerWindow ===
// ─────── ⋆⋅☆⋅⋆ ────────

class RCLoggerWindow : public Window {
  public:

    RCLoggerWindow()
      : cmb_port(nullptr)
      , btn_connect(nullptr),      btn_disc(nullptr)
      , btn_charge(nullptr),       btn_discharge(nullptr)
      , btn_clear(nullptr),        btn_export(nullptr)
      , lbl_tau_measured(nullptr), lbl_tau_theory(nullptr)
      , bCycleActive(false)
    {}

    ~RCLoggerWindow() {
      delete cmb_port;
      delete btn_connect;      delete btn_disc;
      delete btn_charge;       delete btn_discharge;
      delete btn_clear;        delete btn_export;
      delete lbl_tau_measured; delete lbl_tau_theory;
    }
  protected:

    // ─────── ⋆⋅☆⋅⋆ ────────
    //       OnCreate
    // ─────── ⋆⋅☆⋅⋆ ────────
    void OnCreate() override {
      // Connection bar
      new Label(hwnd_self, L"COM Port:", 16, 18, 72, 24);
      cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 92, 14, 130, 200);
      btn_connect = new Button (hwnd_self, L"Connect", ID_BTN_CONNECT, 230, 14, 90, 28);
      btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 328, 14, 100, 28);

      // Cycle controls
      btn_charge = new Button(hwnd_self, L"Charge", ID_BTN_CHARGE, 16, 54, 90, 28);
      btn_discharge = new Button(hwnd_self, L"Discharge", ID_BTN_DISCHARGE, 114, 54, 90, 28);
      btn_clear = new Button(hwnd_self, L"Clear", ID_BTN_CLEAR, 212, 54, 70, 28);
      btn_export = new Button(hwnd_self, L"Export CSV", ID_BTN_EXPORT, 290, 54, 90, 28);

      // Tau readouts
      new Label(hwnd_self, L"τ measured:", 16,  94, 90, 22);
      new Label(hwnd_self, L"τ theory:",   16, 118, 90, 22);
      lbl_tau_measured = new Label(hwnd_self, L"---", 110, 94, 120, 22);
      lbl_tau_theory = new Label(hwnd_self, L"---", 110, 118, 120, 22);

      // Set theoretical tau immediately 
      wchar_t arrTheory[32];
      int iWhole = (int)(TAU_THEORY);
      int iFrac = (int)((TAU_THEORY - iWhole) * 1000);
      wsprintf(arrTheory, L"%d.%03d s", iWhole, iFrac);
      lbl_tau_theory->SetText(arrTheory);

      // Plot panel
      plot.Init(16, 148, 608, 330);

      // Initial button states
      btn_disc->Disable();
      btn_charge->Disable();
      btn_discharge->Disable();
      btn_export->Disable();

      ScanComPorts(cmb_port);
    }

    // ─────── ⋆⋅☆⋅⋆ ────────
    // On Paint
    // ─────── ⋆⋅☆⋅⋆ ────────
    void  OnPaint(HDC hdc) override {
      plot.Draw(hdc);
    }

    // ─────── ⋆⋅☆⋅⋆ ────────
    // OnCommand
    // ─────── ⋆⋅☆⋅⋆ ────────
    void OnCommand(int iControlId, int iNotifCode) override {
      switch(iControlId) {
        case ID_BTN_CONNECT: OnConnect(); break;
        case ID_BTN_DISCONNECT: OnDisconnect(); break;
        
        case ID_BTN_CHARGE:
          port.Write("CHARGE");
          bCycleActive = true;
          btn_charge->Disable();
          btn_discharge->Disable();
          break;
        
        case ID_BTN_DISCHARGE:
          port.Write("DISCHARGE");
          bCycleActive = true;
          btn_charge->Disable();
          btn_discharge->Disable();
          break;
        
        case ID_BTN_CLEAR:
          plot.Clear();
          lbl_tau_measured->SetText(L"---");
          InvalidateRect(hwnd_self, NULL, FALSE);
          break;
        
        case ID_BTN_EXPORT:
        {
          if(plot.iCount == 0) {
            MessageBox(hwnd_self, L"No data to export.", L"Export", MB_OK | MB_ICONWARNING);
            return;
          }

          wchar_t arrPath[MAX_PATH];
          if(!ShowSaveDialog(hwnd_self, L"CSV Files\0*.csv\0ALL Files\0*.*\0", L"csv", arrPath, MAX_PATH))
          return;

          if(!ExportCSV(arrPath, plot.arrTime, plot.arrVolt, plot.iCount)) {
            MessageBox(hwnd_self, L"Export failed.", L"Export", MB_OK | MB_ICONERROR);
          }
          break;
        }
      }
    }

    // ─────── ⋆⋅☆⋅⋆ ────────
    // OnTimer - poll serial, parse sameples and control messages
    // ─────── ⋆⋅☆⋅⋆ ────────
    void OnTimer(int iTimerId) override {
      if(iTimerId != ID_TIMER_POLL) return;
      if(!port.IsOpen()) return;

      char arrRaw[READ_BUF];
      DWORD dwRead = 0;
      port.Read(arrRaw, READ_BUF, dwRead);
      if(dwRead == 0) return;

      arrRaw[dwRead] = '\0';

      // Convert to wide for parsing
      wchar_t arrWide[READ_BUF];
      MultiByteToWideChar(CP_ACP, 0, arrRaw, -1, arrWide, READ_BUF);

      // Split on newlines (multiple lines may arrive in one read)
      wchar_t* pLine = arrWide;
      wchar_t* pEnd = nullptr;

      while((pEnd = wcschr(pLine, L'\n')) != nullptr) {
        *pEnd = L'\0';
        ParseLine(pLine);
        pLine = pEnd + 1;
      }

      InvalidateRect(hwnd_self, NULL, FALSE);
    }

    // ─────── ⋆⋅☆⋅⋆ ────────
    // OnDestroy
    // ─────── ⋆⋅☆⋅⋆ ────────
    void OnDestroy() override {
      KillTimer(hwnd_self, ID_TIMER_POLL);
      port.Close();
    }

  private:

  // ─────── ⋆⋅☆⋅⋆ ────────
  // ParseLine
  // Handles three message types from the Arduino
  //   "T:xxxx,V:x.xxx" - Data sample, add to plot
  //   "DONE, TAU:x.xxxx" - Cycle complete, update tau readout
  //   "CYCLE:..." - status string, ignore silently
  // ─────── ⋆⋅☆⋅⋆ ────────

  /// @param szLine Null-terminated wide string for one serial line
  /// @returns void
  /// @brief Routes a single incoming serial line to the correct handler
  void ParseLine(const wchar_t* szLine) {
    
    // printf("[RX] %S\n", szLine);    // %S = wide string in printf
    // if(wcsstr(szLine, L"V:")) printf("[DEBUG] %f\n", ParseFloat(szLine, L"V:"));

    if(wcsstr(szLine, L"T:") && wcsstr(szLine, L"V:")) {
      float fT = ParseFloat(szLine, L"T:");
      float fV = ParseFloat(szLine, L"V:");
      if(fT >= 0 && fV >= 0) plot.AddSample(fT, fV);
    } else if(wcsstr(szLine, L"DONE")) {
      float fTau = ParseFloat(szLine, L"TAU:");

      if(fTau > 0) {
        wchar_t arrBuf[32];
        int iW = (int)fTau;
        int iF = (int)((fTau - iW) * 1000);
        wsprintf(arrBuf, L"%d.%03d s", iW, iF);
        lbl_tau_measured->SetText(arrBuf);
      } else  {
        lbl_tau_measured->SetText(L"N/A");
      }

      // Cycle complete
      bCycleActive = false;
      btn_charge->Enable();
      btn_discharge->Enable();
      btn_export->Enable();
    }
  }

  // ─────── ⋆⋅☆⋅⋆ ────────
  // OnConnect / OnDisconnect
  // ─────── ⋆⋅☆⋅⋆ ────────
  void OnConnect() {
    if(cmb_port->GetCount() == 0) {
      MessageBox(hwnd_self, L"No COM ports found.", L"Connect", MB_OK | MB_ICONWARNING);
      return;
    }

    wchar_t arrPortName[16];
    cmb_port->GetSelected(arrPortName, 16);

    if(!port.Open(arrPortName)) {
      wchar_t arrMsg[64];
      wsprintf(arrMsg, L"Failed to eopen %2.\nError: %lu", arrPortName, port.GetLastErrorCode());
      MessageBox(hwnd_self, arrMsg, L"Connect", MB_OK | MB_ICONERROR);
      return;
    }

    SetTimer(hwnd_self, ID_TIMER_POLL, POLL_MS, NULL);

    btn_connect->Disable();
    btn_disc->Enable();
    btn_charge->Enable();
    btn_discharge->Enable();
    cmb_port->Disable();
  }

  void OnDisconnect() {
    KillTimer(hwnd_self, ID_TIMER_POLL);
    port.Close();
    
    btn_connect->Enable();
    btn_disc->Disable();
    btn_charge->Disable();
    btn_discharge->Disable();
    btn_export->Disable();
    cmb_port->Enable();

    ScanComPorts(cmb_port);
  }

  // ─────── ⋆⋅☆⋅⋆ ────────
  // Members
  // ─────── ⋆⋅☆⋅⋆ ────────
  SerialPort   port;
  PlotPanel    plot;
  bool         bCycleActive;

  ComboBox*    cmb_port;
  Button*      btn_connect;
  Button*      btn_disc;
  Button*      btn_charge;
  Button*      btn_discharge;
  Button*      btn_clear;
  Button*      btn_export;
  Label*       lbl_tau_measured;
  Label*       lbl_tau_theory;
};

// ─────── ⋆⋅☆⋅⋆ ────────
// WinMain
// ─────── ⋆⋅☆⋅⋆ ────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  
    // Allocate a debug console — remove for final build
    // AllocConsole();
    // freopen("CONOUT$", "w", stdout);
    // freopen("CONOUT$", "w", stderr);
  
  RCLoggerWindow win_main;
  win_main.Create(hInstance, L"RC CIrcuit Time Constant Logger", 720, 520);
  win_main.Show(nCmdShow);
  return win_main.Run();
}