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
  const char* szHdr = "Time_ms,Current_mA\r\n";
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

  void HBridgeWindow::OnPaint(HDC hdc) {
    plot.Draw(hdc);
  }

  void HBridgeWindow::OnCommand(int iControlId, int iNotifCode) {
    switch(iControlId) {
      case ID_BTN_CONNECT:
        OnConnect();
        break;
      case ID_BTN_DISCONNECT:
        OnDisconnect();
        break;
      case ID_BTN_BRAKE:
        port.Write("BRAKE");
        lbl_dir_val->SetText(L"Brake");
        trk_speed->SetPos(0);
        iLastSpeed = 0;
        UpdateSpeedLabel(0);
        break;
      case ID_BTN_COAST:
        port.Write("COAST");
        trk_speed->SetPos(0);
        iLastSpeed = 0;
        UpdateSpeedLabel(0);
        break;
      case ID_BTN_CLEARSTALL:
        port.Write("CLEARSTALL");
        bStallAlertShown = false;
        lbl_stall_status->SetText(L"");
        btn_clearstall->Disable();
        trk_speed->Enable();
        trk_speed->SetPos(0);
        iLastSpeed = 0;
        UpdateSpeedLabel(0);
        break;
      case ID_BTN_SETVPIN:
      {
        wchar_t arrBuf[16];
        edit_vpin->GetText(arrBuf, 16);
        float fVpin = wcstof(arrBuf, nullptr);
        if(fVpin > 0.1f && fVpin < 6.0f) {
          char arrCmd[24];
          int iW = (int)fVpin;
          int iF = (int)((fVpin - iW) * 1000);
          wsprintfA(arrCmd, "SETVPIN:%d.%03d", iW, iF);
          port.Write(arrCmd);
        }
        break;
      }
      case ID_BTN_EXPORT:
      {
        if(plot.iCount == 0) {
          MessageBox(hwnd_self, L"No data to export.", L"Export", MB_OK | MB_ICONWARNING);
          return;
        }
        wchar_t arrPath[MAX_PATH];
        if(!ShowSaveDialog(hwnd_self, L"CSV Files\0*.csv\0ALL Files\0*.*", L"csv", arrPath, MAX_PATH))
          return;
        if(!ExportCSV(arrPath, plot.arrTime, plot.arrCurr, plot.iCount))
          MessageBox(hwnd_self, L"Export failed.", L"Export", MB_OK | MB_ICONERROR);
        break;
      }
    }
  }

  void HBridgeWindow::OnScroll(HWND hwnd_control, int iCode) {
    if(!trk_speed) return;
    if(hwnd_control != trk_speed->GetHandle()) return;
    if(!port.IsOpen()) return;

    int iPos = trk_speed->GetPos();

    // Dead zone
    // Snap to center so motor doesn't creep or whine from stiction
    if(abs(iPos) <= 5) {
      iPos = 0;
      trk_speed->SetPos(0); // TBM_SETPOS does not generate WM_HSCROLL
    }

    if(iPos == iLastSpeed) return;
    iLastSpeed = iPos;
    
    char arrCmd[16];
    wsprintfA(arrCmd, "SPEED:%d", iPos);
    port.Write(arrCmd);

    UpdateSpeedLabel(iPos);
  }

  void HBridgeWindow::OnTimer(int iTimerId) {
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

    while ((pEnd = strchr(pLine, '\n')) != nullptr) {
      *pEnd = '\0';
      int iLen = (int)strlen(pLine);
      if(iLen > 0 && pLine[iLen - 1] == '\r') {
        pLine[iLen - 1] = '\0';
      }

      wchar_t arrWide[READ_BUF];
      MultiByteToWideChar(CP_ACP, 0, pLine, -1, arrWide, READ_BUF);

      if(ParseLine(arrWide)) {
        bRepaint = true;
      }
      pLine = pEnd + 1;
  }

  int iRemaining = iLineBufLen - (int)(pLine - arrLineBuf);
  if(iRemaining > 0) memmove(arrLineBuf, pLine, iRemaining);
  iLineBufLen = iRemaining;
  arrLineBuf[iLineBufLen] = '\0';

  if(bRepaint)
    InvalidateRect(hwnd_self, NULL, FALSE);
}

void HBridgeWindow::OnDestroy() {
  if(port.IsOpen()) {
    port.Write("COAST"); // Stop before exit
  }
  KillTimer(hwnd_self, ID_TIMER_POLL);
  port.Close();
}

bool HBridgeWindow::ParseLine(const wchar_t* szLine) {
  // Telemetry: "SPD:xx,DIR:F,CURR:x.xxx,VCC:x.xxx,STALL:n"
  if(wcsstr(szLine, L"SPD:") && wcsstr(szLine, L"CURR:")) {
    float fSpd = ParseFloat(szLine, L"SPD:");
    float fCurr = ParseFloat(szLine, L"CURR:");
    float fVcc = ParseFloat(szLine, L"VCC:");
    float fStall = ParseFloat(szLine, L"STALL:");

    if(fSpd < 0 || fCurr < 0) return false;

    // Speed
    wchar_t arrBuf[16];
    wsprintf(arrBuf, L"%d%%", (int)fSpd);
    lbl_spd_val->SetText(arrBuf);

    // Direction. read the single char after "DIR:"
    const wchar_t* pDir = wcsstr(szLine, L"DIR:");
    if(pDir) {
      switch(pDir[4]) {
        case L'F': 
          lbl_dir_val->SetText(L"Forward");
          break;
        case L'R':
          lbl_dir_val->SetText(L"Reverse");
          break;
        case L'B':
          lbl_dir_val->SetText(L"Brake");
          break;
        default:
          lbl_dir_val->SetText(L"Coast");
          break;
      }
    }

    // Current in mA
    float fCurr_mA = fCurr * 1000.0f;
    int iW = (int)fCurr_mA;
    int iF = abs((int)((fCurr_mA - iW) * 10));
    wsprintf(arrBuf, L"%d.%01d mA", iW, iF);
    lbl_curr_val->SetText(arrBuf);

    // VCC in V
    if(fVcc > 0.0f) {
      int iVW = (int)fVcc;
      int iVF = (int)((fVcc - iVW) * 1000);
      wsprintf(arrBuf, L"%d.%03d V", iVW, iVF);
      lbl_vcc_val->SetText(arrBuf);
    }

    // Stall flag
    if(fStall > 0.5f && !bStallAlertShown) {
      ShowStallAlert();
    }

    // Current history
    long lNow = (long)GetTickCount();
    if(lRecStart == 0)
      lRecStart = lNow;
    plot.AddSample((float)(lNow - lRecStart), fCurr_mA);

    return true;
  }

  // Stall alert message
  if(wcsstr(szLine, L"ALERT:STALL")) {
    if(!bStallAlertShown)
      ShowStallAlert();
    return false;
  }

  // VCC reading (startup and MEASURE_VCC)
  if(wcsstr(szLine, L"VCC:") && !wcsstr(szLine, L"CURR:")) {
    float fVcc = ParseFloat(szLine, L"VCC:");
    if(fVcc > 0.0f) {
      wchar_t arrBuf[24];
      int iW = (int)fVcc;
      int iF = (int)((fVcc - iW) * 1000);
      wsprintf(arrBuf, L"%d.%03d V", iW, iF);
      lbl_vcc_val->SetText(arrBuf);
    }
    return false;
  }

  // Stall cleared
  if(wcsstr(szLine, L"STALL:CLEARED")) {
    bStallAlertShown = false;
    lbl_stall_status->SetText(L"Stall cleared.");
    return false;
  }

  return false;
}

void HBridgeWindow::ShowStallAlert() {
  bStallAlertShown = true;
  lbl_stall_status->SetText(L"!! STALL !! Motor stopped. Clear Obstruction then click Clear Stall");
  btn_clearstall->Enable();
  trk_speed->Disable();
  trk_speed->SetPos(0);
  iLastSpeed = 0;
  UpdateSpeedLabel(0);

  MessageBox(hwnd_self,
             L"Motor stall detected!\n\n"
             L"The motor has been stopped to prevent damage.\n"
             L"Check for mechanical obstruction, then click Clear Stall.",
             L"Motor Stall", MB_OK | MB_ICONWARNING);
}

void HBridgeWindow::UpdateSpeedLabel(int iSpeed) {
  wchar_t arrBuf[24];
  if(iSpeed == 0) {
    wsprintf(arrBuf, L"0%% COAST");
  } else if(iSpeed > 0) {
    wsprintf(arrBuf, L"%d%% FWD", iSpeed);
  }  else {
    wsprintf(arrBuf, L"%d%% REV", -iSpeed);
  }
  lbl_speed_val->SetText(arrBuf);
}

void HBridgeWindow::OnConnect() {
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
  btn_brake->Enable();
  btn_coast->Enable();
  btn_setvpin->Enable();
  btn_export->Enable();
  trk_speed->Enable();
  cmb_port->Disable();

  port.Write("COAST");
  UpdateSpeedLabel(0);
}

void HBridgeWindow::OnDisconnect()
{
    if (port.IsOpen()) port.Write("COAST");

    KillTimer(hwnd_self, ID_TIMER_POLL);
    port.Close();

    btn_connect->Enable();
    btn_disc->Disable();
    btn_brake->Disable();
    btn_coast->Disable();
    btn_clearstall->Disable();
    btn_setvpin->Disable();
    btn_export->Disable();
    trk_speed->Disable();
    trk_speed->SetPos(0);
    cmb_port->Enable();

    lbl_spd_val->SetText (L"---");
    lbl_dir_val->SetText (L"---");
    lbl_curr_val->SetText(L"---");
    lbl_vcc_val->SetText (L"---");
    lbl_stall_status->SetText(L"");
    UpdateSpeedLabel(0);

    iLastSpeed       = 0;
    bStallAlertShown = false;

    ScanComPorts(cmb_port);
}

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  INITCOMMONCONTROLSEX icc = {};
  icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icc.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icc);

  HBridgeWindow win_main;
  win_main.Create(hInstance, L"H-Bridge DC Motor Controller", 640, 600);
  win_main.Show(nCmdShow);
  return win_main.Run();
}