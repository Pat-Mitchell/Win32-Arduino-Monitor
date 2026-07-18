/// @file HBridgeMotor.cpp
/// @brief H-bridge DC Motor Controller implementation

#include "HBridgeMotor.h"

namespace {
  const COLORREF kCurve = RGB(255, 160, 40); // current curve
  const COLORREF kStall = RGB(200, 50, 50); // stall threshold
  const COLORREF kAxis = RGB(100, 100, 100);
  const COLORREF kText = RGB(140, 140, 140);
}

// ────── ⋆⋅☆⋅⋆ ────────
// ExportCSV
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Write time/current dataset to a CSV file.
/// @param szPath Full path from save dialog
/// @param arrTime Timestamp array in ms
/// @param arrCurr Current array in mA
/// @param iCount Number of samples
/// @return TRUE on success

BOOL ExportCSV(const wchar_t* szPath, const float* arrTime, const float* arrCurr, int iCount) {
  HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE) return FALSE;

  DWORD dw = 0;
  const char* szHdr = "Ttime_ms,Current_mA\r\n";
  WriteFile(hFile, szHdr, lstrlenA(szHdr), &dw, NULL);

  for(int i = 0; i < iCount; i++) {
    char arrRow[64];
    int iW = (int)arrCurr[i];
    int iF = abs((int)((arrCurr[i] - iW) * 10));
    int iLen = wsprintfA(arrRow, "%d,%d.%01d\r\n", (int)arrTime[i], iW, iF);
    WriteFile(hFile, arrRow, iLen, &dw, NULL);
  }

  CloseHandle(hFile);
  return TRUE;
}

// ────── ⋆⋅☆⋅⋆ ────────
// CurrentPlotPanel
// ────── ⋆⋅☆⋅⋆ ────────
void CurrentPlotPanel::Init(int iX, int iY, int iW, int iH) {
  rect_bounds = { iX, iY, iX + iW, iY + iH };
  iPadL = 58;
  iPadT = 20;
  iPadR = 16;
  iPadB = 26;
  Clear();
}

void CurrentPlotPanel::Clear() {
  iCount = 0;
  ZeroMemory(arrTime, sizeof(arrTime));
  ZeroMemory(arrCurr, sizeof(arrCurr));
}

void CurrentPlotPanel::AddSample(float fTime_ms, float fCurr_mA) {
  if(iCount >= MAX_SAMPLES) {
    // Scroll left. Oldest sample falls off the left edge
    memmove(arrTime, arrTime + 1, (MAX_SAMPLES - 1) * sizeof(float));
    memmove(arrCurr, arrCurr + 1, (MAX_SAMPLES - 1) * sizeof(float));
    iCount = MAX_SAMPLES - 1;
  }
  arrTime[iCount] = fTime_ms;
  arrCurr[iCount] = fCurr_mA;
  iCount++;
}

float CurrentPlotPanel::GetYMax() const {
  float fMax = STALL_MA * 1.2f; // Stall line is always visible
  for(int i = 0; i < iCount; i++) {
    if(arrCurr[i] > fMax) {
      fMax = arrCurr[i] * 1.1f;
    }
  }
  return fMax;
}

int CurrentPlotPanel::MapXByIdx(int i) const {
  int iW = (rect_bounds.right - rect_bounds.left) - iPadL - iPadR;
  if(iCount <= 1) {
    return rect_bounds.left + iPadL;
  }
  return rect_bounds.left + iPadL + (int)((float) i / (iCount - 1) * iW);
}

int CurrentPlotPanel::MapY(float fVal, float fYmax) const {
  int iH = (rect_bounds.bottom - rect_bounds.top) - iPadT - iPadB;
  if (fYmax < 1.0f) {
    fYmax = 1.0f;
  }
  return rect_bounds.top + iPadT + iH - (int)((fVal / fYmax) * iH);
}

void CurrentPlotPanel::Draw(HDC hdc) const {
  int iL = rect_bounds.left;
  int iT = rect_bounds.top;
  int iR = rect_bounds.right;
  int iB = rect_bounds.bottom;

  // Background
  HBRUSH hbr = CreateSolidBrush(RGB(20, 20, 20));
  RECT rf = rect_bounds;
  FillRect(hdc, &rf, hbr);
  DeleteObject(hbr);

  float fYmax = GetYMax();

  // Axes 
  HPEN hpen_ax = CreatePen(PS_SOLID, 1, kAxis);
  HPEN hpen_old = (HPEN)SelectObject(hdc, hpen_ax);
  MoveToEx(hdc, iL + iPadL, iT + iPadT, NULL);
  LineTo(hdc, iL + iPadL, iB - iPadB);
  MoveToEx(hdc, iL + iPadL, iB - iPadB, NULL);
  LineTo(hdc, iR - iPadR, iB - iPadB);
  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_ax);

  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, kText);

  // Y axis labels
  wchar_t arrBuf[16];
  int iYtop = (int)fYmax;
  int iYmid = iYtop / 2;
  wsprintf(arrBuf, L"%dmA", iYtop);
  TextOut(hdc, iL + 2, MapY(fYmax, fYmax) - 6, arrBuf, lstrlen(arrBuf));
  wsprintf(arrBuf, L"%dmA", iYmid);
  TextOut(hdc, iL + 2, MapY((float)iYmid, fYmax) - 6, arrBuf, lstrlen(arrBuf));
  TextOut(hdc, iL + 2, MapY(0.0f, fYmax) - 6, L"0mA", 3);

  // X axis time span Label
  if(iCount > 1) {
    int iTspan_s = (int)((arrTime[iCount - 1] - arrTime[0]) / 1000.0f);
    wsprintf(arrBuf, L"%ds", iTspan_s);
    TextOut(hdc, iR - iPadR - 20, iB - iPadB + 6, arrBuf, lstrlen(arrBuf));
    TextOut(hdc, iL + iPadL - 2, iB - iPadB + 6, L"0", 1);
  }

  // Stall threshold line
  HPEN hpen_stall = CreatePen(PS_DASH, 1, kStall); // Dashed red
  SelectObject(hdc, hpen_stall);
  int iYstall = MapY(STALL_MA, fYmax);
  MoveToEx(hdc, iL + iPadL, iYstall, NULL);
  LineTo(hdc, iR - iPadR, iYstall);
  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_stall);
  SetTextColor(hdc, kStall);
  TextOut(hdc, iL + iPadL + 4, iYstall - 14, L"stall threshold (1200mA)", 24);

  // Data curve
  if(iCount < 2) return;

  HPEN hpen_c = CreatePen(PS_SOLID, 2, kCurve);
  SelectObject(hdc, hpen_c);
  MoveToEx(hdc, MapXByIdx(0), MapY(arrCurr[0], fYmax), NULL);
  for(int i = 1; i < iCount; i++) {
    LineTo(hdc, MapXByIdx(i), MapY(arrCurr[i], fYmax));
  }
  SelectObject(hdc, hpen_old);
  DeleteObject(hpen_c);
}

// ────── ⋆⋅☆⋅⋆ ────────
// HBridgeWindow
// ────── ⋆⋅☆⋅⋆ ────────
HBridgeWindow::HBridgeWindow()
  : cmb_port(nullptr)
  , btn_connect(nullptr)
  , btn_disc(nullptr)
  , trk_speed(nullptr)
  , lbl_speed_val(nullptr)
  , btn_brake(nullptr)
  , btn_coast(nullptr)
  , btn_clearstall(nullptr)
  , edit_vpin(nullptr)
  , btn_setvpin(nullptr)
  , btn_export(nullptr)
  , lbl_spd_val(nullptr)
  , lbl_dir_val(nullptr)
  , lbl_curr_val(nullptr)
  , lbl_vcc_val(nullptr)
  , lbl_stall_status(nullptr)
  , iLineBufLen(0)
  , lRecStart(0)
  , iLastSpeed(0)
  , bStallAlertShown(false)
  {
    ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
  }

  HBridgeWindow::~HBridgeWindow() {
    delete cmb_port;
    delete btn_connect;
    delete btn_disc;
    delete trk_speed;
    delete lbl_speed_val;
    delete btn_brake;
    delete btn_coast;
    delete btn_clearstall;
    delete edit_vpin;
    delete btn_setvpin;
    delete btn_export;
    delete lbl_spd_val;
    delete lbl_dir_val;
    delete lbl_curr_val;
    delete lbl_vcc_val;
    delete lbl_stall_status;
  }

  void HBridgeWindow::OnCreate() {
    // Connection bar
    new Label(hwnd_self, L"COM Port:", 16, 18, 72, 24);
    cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 92, 14, 120, 200);
    btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, 220, 14, 90, 28);
    btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 318, 14, 100, 28);
    btn_disc->Disable();
    ScanComPorts(cmb_port);

    // Speed slider. Range: -100 to +100, zero at center
    new Label(hwnd_self, L"Speed:", 16, 64, 56, 22);
    trk_speed = new Trackbar(hwnd_self, ID_TRACKBAR_SPEED, 76, 56, 440, 40, -100, 100);
    trk_speed->SetPos(0);
    trk_speed->SetTickFreq(25); // Ticks at -100, -75, -50, -25, 0, 25... 100
    lbl_speed_val = new Label(hwnd_self, L"0% COAST", 524, 64, 100, 22);
    trk_speed->Disable();

    // Control buttons
    btn_brake = new Button(hwnd_self, L"Brake", ID_BTN_BRAKE, 16, 104, 90, 28);
    btn_coast = new Button(hwnd_self, L"Coast", ID_BTN_COAST, 114, 104, 90, 28);
    btn_clearstall = new Button(hwnd_self, L"Clear Stall", ID_BTN_CLEARSTALL, 212, 104, 100, 28);
    btn_brake->Disable();
    btn_coast->Disable();
    btn_clearstall->Disable();

    // V_pin calibration and export
    new Label(hwnd_self, L"V_pin:", 16, 142, 100, 22);
    edit_vpin = new TextInput(hwnd_self, ID_EDIT_VPIN, 120, 138, 70, 28);
    btn_setvpin = new Button(hwnd_self, L"Set", ID_BTN_SETVPIN, 198, 138, 60, 28);
    btn_export = new Button(hwnd_self, L"Export CSV", ID_BTN_EXPORT, 470, 138, 100, 28);
    edit_vpin->SetText(L"4.85");
    btn_setvpin->Disable();
    btn_export->Disable();

    // Divider
    new Label(hwnd_self, L"──────────────────────────────────────────", 16, 174, 600, 18);

    // Readout rows
    new Label(hwnd_self, L"Speed:", 16, 192, 90, 22);
    new Label(hwnd_self, L"Direction:", 16, 216, 90, 22);
    new Label(hwnd_self, L"Current:", 16, 240, 90, 22);
    new Label(hwnd_self, L"VCC:", 16, 264, 90, 22);

    lbl_spd_val = new Label(hwnd_self, L"---", 110, 192, 120, 22);
    lbl_dir_val = new Label(hwnd_self, L"---", 110, 216, 120, 22);
    lbl_curr_val = new Label(hwnd_self, L"---", 110, 240, 120, 22);
    lbl_vcc_val = new Label(hwnd_self, L"---", 110, 264, 120, 22);

    lbl_stall_status = new Label(hwnd_self, L"", 250, 216, 350, 22);

    // Current history plot
    new Label(hwnd_self, L"Current history:", 16, 294, 130, 22);
    plot.Init(16, 316, 592, 240);
  }