/// @file wheatstone.cpp
/// @brief Wheatstone bridge temperature monitor.
///   Displays live V_a, V_b, V_diff, R_therm, and temperature.
///   Plots temperature history over time. Supports calibration
///   of both the arduino and bridge

#include "../../pch.h"
#include "../../Utils/Utils.h"
#include "../../wrappers/win32Wrappers/Window.h"
#include "../../wrappers/win32Wrappers/Button.h"
#include "../../wrappers/win32Wrappers/Label.h"
#include "../../wrappers/win32Wrappers/ComboBox.h"
#include "../../wrappers/serialPortWrappers/SerialPort.h"

#define ID_COMBO_PORT 101
#define ID_BTN_CONNECT 102
#define ID_BTN_DISCONNECT 103
#define ID_BTN_CALIBRATE 104
#define ID_BTN_RESET_CAL 105
#define ID_BTN_EXPORT 106
#define ID_TIMER_POLL 1

#define POLL_MS 200
#define READ_BUF 512
#define MAX_SAMPLES 512

// ────── ⋆⋅☆⋅⋆ ────────
// ReadoutPanel
// Label + value + unit display row
// ────── ⋆⋅☆⋅⋆ ────────

struct ReadoutPanel {
  Label* lbl_name = nullptr;
  Label* lbl_value = nullptr;
  Label* lbl_unit = nullptr;

  /// @brief Creates the three labels for one readout row
  /// @param hwnd_parent Parent window
  /// @param szName Row label
  /// @param szUnit Unit string
  /// @param iX Left position
  /// @param iY Top position
  void Create(HWND hwnd_parent, const wchar_t* szName, const wchar_t* szUnit, int iX, int iY) {
    lbl_name = new Label(hwnd_parent, szName, iX, iY, 110, 22);
    lbl_value = new Label(hwnd_parent, L"---", iX + 115, iY, 100, 22);
    lbl_unit = new Label(hwnd_parent, szUnit, iX + 220, iY, 60, 22);
  }

  void SetValue(const wchar_t* szValue) {
    if(lbl_value) lbl_value->SetText(szValue);
  }

  void Destroy() {
    delete lbl_name;
    delete lbl_value;
    delete lbl_unit;
    lbl_name = lbl_value = lbl_unit = nullptr;
  }
};

// ────── ⋆⋅☆⋅⋆ ────────
// PlotPanel
// temperature history over time
// ────── ⋆⋅☆⋅⋆ ────────
struct PlotPanel {
  float arrTime[MAX_SAMPLES];
  float arrTemp[MAX_SAMPLES];
  int iCount;
  int iPadL, iPadT, iPadR, iPadB;
  RECT rect_bounds;

  void Init(int iX, int iY, int iW, int iH) {
    rect_bounds = {iX, iY, iX + iW, iY + iH };
    iPadL = 50; iPadT = 16; iPadR = 16; iPadB = 36;
    Clear();
  }

  void AddSample(float fT, float fTemp) {
    if(iCount >= MAX_SAMPLES) return;
    arrTime[iCount] = fT;
    arrTemp[iCount] = fTemp;
    iCount++;
  }

  void Clear() {
    iCount = 0;
    ZeroMemory(arrTime, sizeof(arrTime));
    ZeroMemory(arrTemp, sizeof(arrTemp));
  }

  /// @brief Finds min and max temperature in the dataset for Y axis scaling
  ///   Adds a small margin above and below. Falls back to 15-35 C when
  ///   the dataset is empty.
  /// @param fMin 
  /// @param fMax 
  void GetTempRange(float& fMin, float& fMax) const {
    if(iCount) { fMin = 15.0f; fMax = 35.0f; return; }

    fMin = arrTemp[0]; fMax = arrTemp[0];
    for(int i = 1; i < iCount; i++) {
      if(arrTemp[i] < fMin) fMin = arrTemp[i];
      if(arrTemp[i] > fMax) fMax = arrTemp[i];
    }

    // Add 2 C margin each side so the curve doesn't sit on the axes
    fMin -= 2.0f;
    fMax += 2.0f;
  }

  int MapX(float fT, float fTmax) const {
    int iW = (rect_bounds.right - rect_bounds.left) - iPadL - iPadR;
    return rect_bounds.left + iPadL + (int)((fT / fTmax) * iW);
  }

  int MapY(float fTemp, float fTmin, float fTmax) const {
     int iH = (rect_bounds.bottom - rect_bounds.top) - iPadT - iPadB;
     float fRange = fTmax - fTmin;
     if(fRange < 0.001f) fRange = 1.0f;
     return rect_bounds.top + iPadT + iH - (int)(((fTemp - fTmin) / fRange) * iH);
  }

  /// @brief Renders the temperature history plot
  /// @param hdc device context
  /// @param bCalibrated Draws a note if calibration has not been done
  void Draw(HDC hdc, bool bCalibrated) const {
    int iL = rect_bounds.left;
    int iT = rect_bounds.top;
    int iR = rect_bounds.right;
    int iB = rect_bounds.bottom;

    // Background
    HBRUSH hbr = CreateSolidBrush(RGB(20, 20, 20));
    RECT rf = rect_bounds;
    FillRect(hdc, &rf, hbr);
    DeleteObject(hbr);

    float fTmin, fTmax;
    GetTempRange(fTmin, fTmax);

    // Axes
    HPEN hpen_ax = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    SelectObject(hdc, hpen_ax);
    MoveToEx(hdc, iL + iPadL, iT + iPadT, NULL);
    LineTo(hdc, iL + iPadL, iB - iPadB);
    MoveToEx(hdc, iL + iPadL, iB - iPadB, NULL);
    LineTo(hdc, iR - iPadR, iB - iPadB);
    DeleteObject(hpen_ax);

    // Y axis labels. min, mid, max temperature
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(140, 140, 140));

    float fTmid = (fTmin + fTmax) / 2.0f;

    wchar_t arrBuf[16];
    wsprintf(arrBuf, L"%.0f\xB0", fTmax); // (degree Celsius)
    TextOut(hdc, iL + iPadL - 40, MapY(fTmax, fTmin, fTmax) - 6, arrBuf, lstrlen(arrBuf));
    wsprintf(arrBuf, L"%.0f\xB0", fTmid);
    TextOut(hdc, iL + iPadL - 40, MapY(fTmid, fTmin, fTmax) - 6, arrBuf, lstrlen(arrBuf));
    wsprintf(arrBuf, L"%.0f\xB0", fTmin);
    TextOut(hdc, iL + iPadL - 40, MapY(fTmin, fTmin, fTmax) - 6, arrBuf, lstrlen(arrBuf));

    // Horizontal mid-line (dashed)
    HPEN hpen_mid = CreatePen(PS_DASH, 1, RGB(60, 60, 60));
    SelectObject(hdc, hpen_mid);
    MoveToEx(hdc, iL + iPadL, MapY(fTmid, fTmin, fTmax), NULL);
    LineTo(hdc, iR - iPadR, MapY(fTmid, fTmin, fTmax));
    DeleteObject(hpen_mid);

    // Uncalibrated warning
    if(!bCalibrated) {
      SetTextColor(hdc, RGB(20, 140, 40));
      TextOut(hdc, iL + iPadL + 8, iT + iPadT + 4, L"Uncalibrated", 12);
    }

    // Data curve
    if(iCount < 2) return;

    float fTmax_x = arrTime[iCount - 1];
    if(fTmax_x < 1.0f) fTmax_x = 1.0f;

    HPEN hpen_curve = CreatePen(PS_SOLID, 2, RGB(80, 200, 140));
    SelectObject(hdc, hpen_curve);
    MoveToEx(hdc, MapX(arrTime[0], fTmax_x), MapY(arrTemp[0], fTmin, fTmax), NULL);
    for(int i = 1; i < iCount; i++) {
      LineTo(hdc, MapX(arrTime[i], fTmax_x), MapY(arrTemp[i], fTmin, fTmax));
    }
    DeleteObject(hpen_curve);

    // X axis time label
    wsprintf(arrBuf, L"%ds", (int)(fTmax_x / 1000));
    SetTextColor(hdc, RGB(140, 140, 140));
    TextOut(hdc, iR - iPadR - 24, iB - iPadB + 6, arrBuf, lstrlen(arrBuf));
  }
};

// ────── ⋆⋅☆⋅⋆ ────────
// ExportCSV
// Time / temperature with bridge voltages
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Exports the full bridge dataset withh all five measured values.
/// @param szPath full file path
/// @param arrTime Time array in ms
/// @param arrTemp Temperature array in celsius
/// @param iCount Number of samples
/// @return TRUE on success
BOOL ExportCSV(const wchar_t* szPath, const float* arrTime, const float* arrTemp, int iCount) {
  HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if(hFile == INVALID_HANDLE_VALUE) return FALSE;

  DWORD dwWritten = 0;
  const char* szHdr = "TIME_ms,TEMP_C\r\n";
  WriteFile(hFile,szHdr, lstrlenA(szHdr), &dwWritten, NULL);

  for(int i = 0; i < iCount; i++) {
    char arrRow[64];
    int iW = (int)arrTemp[i];
    int iF = abs((int)((arrTemp[i] - (int)arrTemp[i]) * 100));
    int iLen = wsprintfA(arrRow, "%d,%d.%02d\r\n", (int)arrTime[i], iW, iF);
    WriteFile(hFile, arrRow, iLen, &dwWritten, NULL);
  }

  CloseHandle(hFile);
  return TRUE;
}

// ────── ⋆⋅☆⋅⋆ ────────
// WheatstoneWindow
// ────── ⋆⋅☆⋅⋆ ────────
class WheatstoneWindow : public Window {
  public:
    WheatstoneWindow()
      : cmb_port(nullptr)
      , btn_connect(nullptr)
      , btn_disc(nullptr)
      , btn_calibrate(nullptr)
      , btn_reset_cal(nullptr)
      , btn_export(nullptr)
      , lbl_cal_status(nullptr)
      , lbl_vcc(nullptr)
      , bCalibrated(false)
      , iLineBufLen(0)
      , lRecStart(0)
    {
      ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
    }

    ~WheatstoneWindow() {
      delete cmb_port;
      delete btn_connect;
      delete btn_disc;
      delete btn_calibrate;
      delete btn_reset_cal;
      delete btn_export;
      delete lbl_cal_status;
      delete lbl_vcc;

      panel_va.Destroy();
      panel_vb.Destroy();
      panel_vdiff.Destroy();
      panel_rtherm.Destroy();
      panel_temp.Destroy();
    }

  protected:
    // ────── ⋆⋅☆⋅⋆ ────────
    // OnCreate
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnCreate() override {
      // Connection bar
      new Label(hwnd_self, L"COM Port:", 16, 18, 72, 24);
      cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 92, 14, 130, 200);
      btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, 230, 14, 90, 28);
      btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 328, 14, 100, 28);
      btn_disc->Disable();
      ScanComPorts(cmb_port);

      // Calibration bar
      btn_calibrate = new Button(hwnd_self, L"Calibrate", ID_BTN_CALIBRATE, 16, 52, 90, 28);
      btn_reset_cal = new Button(hwnd_self, L"Reset Cal", ID_BTN_RESET_CAL, 114, 52, 90, 28);
      btn_export = new Button(hwnd_self, L"Export CSV", ID_BTN_EXPORT, 470, 52, 110, 28);
      lbl_cal_status = new Label(hwnd_self, L"Status: Uncalibrated", 212, 58, 250, 22);
      lbl_vcc = new Label(hwnd_self, L"VCC: ---", 362, 58, 80, 22);

      btn_calibrate->Disable();
      btn_reset_cal->Disable();
      btn_export->Disable();

      // Divider
      new Label(hwnd_self, L"──────────────────────────────────────", 16, 88, 570, 20);

      // Readout panels
      panel_va.Create(hwnd_self, L"Voltage A:", L"V", 16, 108);
      panel_vb.Create(hwnd_self, L"Voltage B:", L"V", 16, 132);
      panel_vdiff.Create(hwnd_self, L"Differential:", L"V", 16, 156);
      panel_rtherm.Create(hwnd_self, L"R thermistor:", L"\x03A9", 16, 180); // Ohms
      panel_temp.Create(hwnd_self, L"Temperature:", L"\xB0""C", 16, 212); // (degrees) C

      // Divider
      new Label(hwnd_self, L"──────────────────────────────────────", 16, 242, 570, 20);

      plot.Init(16, 266, 592, 250);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnPaint
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnPaint(HDC hdc) override {
      plot.Draw(hdc, bCalibrated);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnCommand
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnCommand(int iControlId, int iNotifCode) override {
      switch(iControlId) {
        case ID_BTN_CONNECT:
          OnConnect();
          break;
        case ID_BTN_DISCONNECT:
          OnDisconnect();
          break;
        case ID_BTN_CALIBRATE:
          port.Write("CALIBRATE");
          lbl_cal_status->SetText(L"Status: Calibrating...");
          break;
        case ID_BTN_RESET_CAL:
          port.Write("RESET");
          bCalibrated = false;
          lbl_cal_status->SetText(L"Status: Uncalibrated");
          InvalidateRect(hwnd_self, NULL, FALSE);
          break;
        case ID_BTN_EXPORT: {
          if(plot.iCount == 0) {
            MessageBox(hwnd_self, L"No date to export.", L"Export", MB_OK | MB_ICONWARNING);
            return;
          }
          wchar_t arrPath[MAX_PATH];
          if(!ShowSaveDialog(hwnd_self, L"CSV Files\0*.csv\0All Files\0*.*\0", L"csv", arrPath, MAX_PATH))
            return;
          if(!ExportCSV(arrPath, plot.arrTime, plot.arrTemp, plot.iCount))
            MessageBox(hwnd_self, L"Export failed.", L"Export", MB_OK | MB_ICONERROR);
          break;
        }
      }
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnTimer
    // Serial polling with persistent line buffer
    // ────── ⋆⋅☆⋅⋆ ────────
    void OnTimer(int iTimerId) override {
      if(iTimerId != ID_TIMER_POLL) return;
      if(!port.IsOpen()) return;

      char arrRaw[READ_BUF];
      DWORD dwRead = 0;
      port.Read(arrRaw, READ_BUF, dwRead);
      if(dwRead == 0) return;

      if(iLineBufLen + (int)dwRead >= (int)sizeof(arrLineBuf)) {
        iLineBufLen = 0;
        ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
      }

      memcpy(arrLineBuf + iLineBufLen, arrRaw, dwRead);
      iLineBufLen += (int)dwRead;
      arrLineBuf[iLineBufLen] = '\0';

      char* pLine = arrLineBuf;
      char* pEnd = nullptr;
      bool bRepaint = false;

      while((pEnd = strchr(pLine, '\n')) != nullptr) {
        *pEnd = '\0';
        int iLen = (int)strlen(pLine);
        if(iLen > 0 && pLine[iLen - 1] == '\r')
          pLine[iLen - 1] = '\0';
        
        wchar_t arrWide[READ_BUF];
        MultiByteToWideChar(CP_ACP, 0, pLine, -1, arrWide, READ_BUF);

        if(ParseLine(arrWide)) bRepaint = true;

        pLine = pEnd + 1;
      }

      int iRemaining = iLineBufLen - (int)(pLine - arrLineBuf);
      if(iRemaining > 0)
        memmove(arrLineBuf, pLine, iRemaining);
      iLineBufLen = iRemaining;
      arrLineBuf[iLineBufLen] = '\0';

      if(bRepaint)
        InvalidateRect(hwnd_self, NULL, FALSE);
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
    // ParseLine
    // Handles three message formats:
    //   "VA:...,VB:...,VDIFF:...,RTHERM:...,:TEMP:...,CAL:n" <- data
    //   "CAL:OK,OFFSET:x.xx" <- Calibration
    //   "VCC:x.xxx" <- supply reading
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Routes incoming serial lines to the appropriatehandler.
    /// @param szLine Wide string for one complete serial line
    /// @return TRUE if a repaint is needed
    bool ParseLine(const wchar_t* szLine) {
      // Data line
      if(wcsstr(szLine, L"VA:") && wcsstr(szLine, L"TEMP:")) {
        float fVA = ParseFloat(szLine, L"VA:");
        float fVB = ParseFloat(szLine, L"VB:");
        float fVdiff = ParseFloat(szLine, L"VDIFF:");
        float fRt = ParseFloat(szLine, L"RTHERM:");
        float fTemp = ParseFloat(szLine, L"TEMP:");
        float fCal = ParseFloat(szLine, L"CAL:");

        if(fVA < 0 || fVB < 0 || fTemp < -273.0) return false;

        bCalibrated = (fCal > 0.5f);

        UpdateReadout(panel_va, fVA, 3, L"");
        UpdateReadout(panel_vb, fVB, 3, L"");
        UpdateReadout(panel_vdiff, fVdiff, 3, L"");
        UpdateReadout(panel_rtherm, fRt, 0, L"");
        UpdateReadout(panel_temp, fTemp, 2, L"");

        // Add to temperature history
        long lNow = (long)GetTickCount();
        if(lRecStart == 0) lRecStart = lNow;
        plot.AddSample((float)(lNow - lRecStart), fTemp);

        return true;
      }

      // Calibration acknowledgement
      if(wcsstr(szLine,L"CAL:OK")) {
        float fOffset = ParseFloat(szLine, L"OFFSET:");
        bCalibrated = true;

        wchar_t arrBuf[64];
        int iW = (int)fOffset;
        int iF = abs((int)((fOffset - (int)fOffset) * 100));
        wsprintf(arrBuf, L"Status: Calibrated (offset%d.%02d\xB0""C)", iW, iF);
        lbl_cal_status->SetText(arrBuf);
        return true;
      }

      // VCC measurement
      if(wcsstr(szLine, L"VCC:")) {
        float fVcc = ParseFloat(szLine, L"VCC:");
        if(fVcc > 0) {
          wchar_t arrBuf[32];
          int iW = (int)fVcc;
          int iF = (int)((fVcc - iW) * 1000);
          wsprintf(arrBuf, L"VCC: %d.%03d V", iW, iF);
          lbl_vcc->SetText(arrBuf);
        }
        return false;
      }
      return false;
    }

    /// @brief Formats a float to a given number of decimal places and updates
    ///   the corresponding ReadoutPanel
    /// @param panel Target Panel
    /// @param fVal Value to display
    /// @param iDec Decimal places 
    /// @param szExtra Append to the value string if needed
    void UpdateReadout(ReadoutPanel& panel, float fVal, int iDec, const wchar_t* szExtra) {
      wchar_t arrBuf[32];
      int iW = (int)fVal;
      int iF = 0;

      switch(iDec) {
        case 0: 
          wsprintf(arrBuf, L"%d", iW); break;
        case 2:
          iF = abs((int)((fVal - iW) * 100));
          wsprintf(arrBuf, L"%d.%02d", iW, iF);
          break;
        case 3:
        default:
          iF = abs((int)((fVal - iW) * 1000));
          wsprintf(arrBuf, L"%d.%03d", iW, iF);
          break;
      }
      panel.SetValue(arrBuf);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // OnConnect / OnDisconnect
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
        wsprintf(arrMsg, L"Failed to open %s.\nError: %lu", arrPortName, port.GetLastErrorCode());
        MessageBox(hwnd_self, arrMsg, L"Connect", MB_OK | MB_ICONERROR);
        return;
      }

      SetTimer(hwnd_self, ID_TIMER_POLL, POLL_MS, NULL);
      lRecStart = 0;

      btn_connect->Disable();
      btn_disc->Enable();
      btn_calibrate->Enable();
      btn_reset_cal->Enable();
      btn_export->Enable();
      cmb_port->Disable();
    }

    void OnDisconnect() {
      KillTimer(hwnd_self, ID_TIMER_POLL);
      port.Close();

      btn_connect->Enable();
      btn_disc->Disable();
      btn_calibrate->Disable();
      btn_reset_cal->Disable();
      cmb_port->Enable();

      panel_va.SetValue(L"---");
      panel_vb.SetValue(L"---");
      panel_vdiff.SetValue(L"---");
      panel_rtherm.SetValue(L"---");
      panel_temp.SetValue(L"---");
      lbl_vcc->SetText(L"VCC: ---");

      ScanComPorts(cmb_port);
    }

    // ────── ⋆⋅☆⋅⋆ ────────
    // Members
    // ────── ⋆⋅☆⋅⋆ ────────
    SerialPort port;
    PlotPanel plot;

    char arrLineBuf[READ_BUF * 4];
    int iLineBufLen;
    long lRecStart;
    bool bCalibrated;

    ComboBox* cmb_port;
    Button* btn_connect;
    Button* btn_disc;
    Button* btn_calibrate;
    Button* btn_reset_cal;
    Button* btn_export;
    Label* lbl_cal_status;
    Label* lbl_vcc;

    ReadoutPanel panel_va;
    ReadoutPanel panel_vb;
    ReadoutPanel panel_vdiff;
    ReadoutPanel panel_rtherm;
    ReadoutPanel panel_temp;
};

// ────── ⋆⋅☆⋅⋆ ────────
// WinMain
// ────── ⋆⋅☆⋅⋆ ────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  WheatstoneWindow win_main;
  win_main.Create(hInstance, L"Wheatstone Bridge - Temperature Monitor", 640, 580);
  win_main.Show(nCmdShow);
  return win_main.Run();
}