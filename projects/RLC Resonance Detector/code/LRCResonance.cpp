#include "LRCResonance.h"

// ────── ⋆⋅☆⋅⋆ ────────
// ExportCSV
// ────── ⋆⋅☆⋅⋆ ────────

BOOL ExportCSV(const wchar_t* szPath, const float* arrFreq, const float* arrV, int iCount) {
  HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE) return FALSE;

  DWORD dw = 0;
  const char* szHdr = "Frequency_Hz,VOltage_V\r\n";
  WriteFile(hFile, szHdr, lstrlenA(szHdr), &dw, NULL);

  for(int i = 0; i < iCount; i++) {
    char arrRow[64];
    int iW = (int)arrV[i];
    int iF = (int)((arrV[i] - iW) * 10000);
    int iLen = wsprintfA(arrRow, "%d,%d.%04d\r\n", (int)arrFreq[i], iW, iF);
    WriteFile(hFile, arrRow, iLen, &dw, NULL);
  }

  CloseHandle(hFile);
  return TRUE;
}

// ────── ⋆⋅☆⋅⋆ ────────
// LRCWindow
// ────── ⋆⋅☆⋅⋆ ────────

LRCWindow::LRCWindow()
  : cmb_port(nullptr) 
  , btn_connect(nullptr)
  , btn_disc(nullptr)
  , edit_fstart(nullptr)
  , edit_fend(nullptr)
  , edit_steps(nullptr)
  , btn_sweep(nullptr)
  , btn_stop(nullptr)
  , edit_lval(nullptr)
  , edit_cval(nullptr)
  , edit_vpin(nullptr)
  , btn_setvpin(nullptr)
  , prog_sweep(nullptr)
  , btn_export(nullptr)
  , lbl_f0_meas(nullptr)
  , lbl_f0_theory(nullptr)
  , lbl_error(nullptr)
  , lbl_status(nullptr)
  , lbl_vcc(nullptr)
  , bSweepActive(false)
  , iExpectedSteps(0)
  , iReceivedSteps(0)
  , fVpin(4.85f)
  , iLineBufLen(0)
{
  ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
}

LRCWindow::~LRCWindow() {
  delete cmb_port;
  delete btn_connect;
  delete btn_disc;
  delete edit_fstart;
  delete edit_fend;
  delete edit_steps;
  delete btn_sweep;
  delete btn_stop;
  delete edit_lval;
  delete edit_cval;
  delete edit_vpin;
  delete btn_setvpin;
  delete prog_sweep;
  delete btn_export;
  delete lbl_f0_meas;
  delete lbl_f0_theory;
  delete lbl_error;
  delete lbl_status;
  delete lbl_vcc;
}

void LRCWindow::OnCreate() {
  // Connection bar
  new Label(hwnd_self, L"COM Port:", 16, 18, 72, 24);
  cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 92, 14, 120, 200);
  btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, 220, 14, 90, 28);
  btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 318, 14, 100, 28);
  btn_disc->Disable();
  ScanComPorts(cmb_port);

  // Sweep config ro
  new Label(hwnd_self, L"f start (Hz):", 16, 56, 88, 22);
  new Label(hwnd_self, L"f end (Hz):", 180, 56, 76, 22);
  new Label(hwnd_self, L"Steps:", 330, 56, 46, 22);

  edit_fstart = new TextInput(hwnd_self, ID_EDIT_FSTART, 108, 52, 68, 28);
  edit_fend = new TextInput(hwnd_self, ID_EDIT_FEND, 260, 52, 68, 28);
  edit_steps = new TextInput(hwnd_self, ID_EDIT_STEPS, 380, 52, 50, 28);
  btn_sweep = new Button(hwnd_self, L"Start Sweep", ID_BTN_SWEEP, 438, 52, 100, 28);
  btn_stop = new Button(hwnd_self, L"Stop", ID_BTN_STOP, 546, 52, 70, 28);

  edit_fstart->SetText(L"5000");
  edit_fend->SetText(L"200000");
  edit_steps->SetText(L"100");
  btn_sweep->Disable();
  btn_stop->Disable();

  // Component values and V_pin row
  new Label(hwnd_self, L"L (µH):", 16, 94, 58, 22);
  new Label(hwnd_self, L"C (nF):", 148, 94, 58, 22);
  new Label(hwnd_self, L"V_pin:", 280, 94, 48, 22);

  edit_lval = new TextInput(hwnd_self, ID_EDIT_LVAL, 78, 90, 62, 28);
  edit_cval = new TextInput(hwnd_self, ID_EDIT_CVAL, 210, 90, 62, 28);
  edit_vpin = new TextInput(hwnd_self, ID_EDIT_VPIN, 332, 90, 72, 28);
  btn_setvpin = new Button(hwnd_self, L"Set V_pin", ID_BTN_SETVPIN, 412, 90, 84,28);

  edit_lval->SetText(L"100");
  edit_cval->SetText(L"100");
  edit_vpin->SetText(L"4.85");
  btn_setvpin->Disable();

  // Progress bar. Full width
  prog_sweep = new ProgressBar(hwnd_self, ID_PROGRESS, 16, 128, 608, 18);

  // Results row
  new Label(hwnd_self, L"f\u2080 measured:", 16, 158, 90, 22);
  new Label(hwnd_self, L"f\u2080 theory:", 218, 158, 74, 22);
  new Label(hwnd_self, L"Error:", 416, 158, 46, 22);

  lbl_f0_meas = new Label(hwnd_self, L"---", 110, 158, 104, 22);
  lbl_f0_theory = new Label(hwnd_self, L"---", 296, 158, 116, 22);
  lbl_error = new Label(hwnd_self, L"---", 466, 158, 80, 22);

  // Status and VCC row
  lbl_vcc = new Label(hwnd_self, L"VCC: ---", 16, 185, 130, 22);
  lbl_status = new Label(hwnd_self, L"Disconnected", 152, 185, 440, 22);

  // Export button
  btn_export = new Button(hwnd_self, L"Export CSV", ID_BTN_EXPORT, 16, 212, 100, 28);
  btn_export->Disable();

  // Frequency response plot
  plot.Init(16, 248, 608, 300);

  // Calculate and display theoretical f0, from default L/C values
  UpdateTheoreticalF0();
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnPaint
// ────── ⋆⋅☆⋅⋆ ────────
void LRCWindow::OnPaint(HDC hdc) {
  plot.Draw(hdc, fVpin);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnCommand
// ────── ⋆⋅☆⋅⋆ ────────
void LRCWindow::OnCommand(int iControlId, int NotifCode) {
  switch(iControlId) {
    case ID_BTN_CONNECT: 
      OnConnect();
      break;
    case ID_BTN_DISCONNECT:
      OnDisconnect();
      break;
    case ID_BTN_SWEEP:
      StartSweep();
      break;
    case ID_BTN_STOP:
      port.Write("STOP");
      lbl_status->SetText(L"Stopping...");
      break;
    case ID_BTN_SETVPIN: {
      wchar_t arrBuf[16];
      edit_vpin->GetText(arrBuf, 16);
      fVpin = wcstof(arrBuf, nullptr);
      if(fVpin < 0.1f || fVpin > 6.0f) {
        fVpin = 4.85f;
      }

      // Send to Arduino -> convert to narrow string
      char arrCmd[24];
      int iW = (int)fVpin;
      int iF = (int)((fVpin - iW) * 1000);
      wsprintfA(arrCmd, "STEPINV:%d.%03d", iW, iF);
      port.Write(arrCmd);

      InvalidateRect(hwnd_self, NULL, FALSE);
      break;
    }
    case ID_BTN_EXPORT: {
      if(plot.iCount == 0) {
        MessageBox(hwnd_self, L"No sweep data to export.", L"Export", MB_OK | MB_ICONWARNING);
        return;
      }
      wchar_t arrPath[MAX_PATH];
      if(!ShowSaveDialog(hwnd_self, L"CSV Files\0*.csv\0All Files\0*.*\0", L"csv", arrPath, MAX_PATH)) {
        return;
      }
      if(!ExportCSV(arrPath, plot.arrFreq, plot.arrV, plot.iCount)) {
        MessageBox(hwnd_self, L"Export failed.", L"Export", MB_OK | MB_ICONERROR);
      }
      break;
    }
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnTimer
// Serial polling with persistent line buffer
// ────── ⋆⋅☆⋅⋆ ────────
void LRCWindow::OnTimer(int iTimerId) {
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
  iLineBufLen += (int) dwRead;
  arrLineBuf[iLineBufLen] = '\0';

  char* pLine = arrLineBuf;
  char* pEnd = nullptr;
  bool bRepaint = false;

  while ((pEnd = strchr(pLine, '\n')) != nullptr) {
    *pEnd = '\0';
    int iLen = (int)strlen(pLine);
    if(iLen > 0 && pLine[iLen - 1] == '\r') {
      pLine[iLen -1];
    }

    wchar_t arrWide[READ_BUF];
    MultiByteToWideChar(CP_ACP, 0, pLine, -1, arrWide, READ_BUF);

    if(ParseLine(arrWide)) {
      bRepaint = true;
    }
  }

  int iRemaining = iLineBufLen = (int)(pLine - arrLineBuf);
  if(iRemaining > 0) {
    memmove(arrLineBuf, pLine, iRemaining);
  }
  iLineBufLen = iRemaining;
  arrLineBuf[iLineBufLen] = '\0';

  if(bRepaint) {
    InvalidateRect(hwnd_self, NULL, FALSE);
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnDestroy
// ────── ⋆⋅☆⋅⋆ ────────
void LRCWindow::OnDestroy() {
  KillTimer(hwnd_self, ID_TIMER_POLL);
  port.Close();
}

// ────── ⋆⋅☆⋅⋆ ────────
// ParseLine
// ────── ⋆⋅☆⋅⋆ ────────
// Message types:
//   "FREQ:x,VPEAK:y"  <-Sweep data point
//   "SWEEP:DONE"      <-Sweep complete
//   "SWEEP:STOPPED"   <-Sweep aborted
//   "VCC:x"           <-Supply voltage reading
//   "VPIN:x"          <-Pin voltage reading
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Routes one incoming serial line to the appropriate handler.
/// @param szLine Wide string for one complete line
/// @return TRUE if a repaint is needed
bool LRCWindow::ParseLine(const wchar_t* szLine) {
  // Sweep data point
  if(wcsstr(szLine, L"FREQ:") && wcsstr(szLine, L"VPEAK:")) {
    float fFreq = ParseFloat(szLine, L"FREQ:");
    float fVpeak = ParseFloat(szLine, L"VPEAK:");

    if(fFreq > 0 && fVpeak >=0) {
      plot.AddSample(fFreq, fVpeak);
      iReceivedSteps++;

      // Update progress bar
      if(iExpectedSteps > 0)
        prog_sweep->SetPos((iReceivedSteps * 100) / iExpectedSteps);

      // Update status periodically
      if(iReceivedSteps % 10 == 0) {
        wchar_t arrBuf[48];
        wsprintf(arrBuf, L"Sweeping... %d / %d steps", iReceivedSteps, iExpectedSteps);
        lbl_status->SetText(arrBuf);
      }
    }
    return true;
  }

  // Sweep complete
  if(wcsstr(szLine, L"SWEEP:DONE") || wcsstr(szLine, L"SWEEP:STOPPED")) {
    bSweepActive = false;

    btn_sweep->Enable();
    btn_stop->Disable();
    btn_setvpin->Enable();
    btn_export->Enable();

    bool bDone = (wcsstr(szLine, L"DONE") != nullptr);
    lbl_status->SetText(bDone ? L"Sweep complete." : L"Sweep stopped.");

    prog_sweep->SetPos(bDone ? 100 : prog_sweep->GetHandle() ? (int)SendMessage(prog_sweep->GetHandle(), PBM_GETPOS, 0, 0) : 0);

    // Update f0 measured label from plot peak
    float fF0 = plot.GetPeakFreq();
    if(fF0 > 0) {
      wchar_t arrBuf[32];
      wsprintf(arrBuf, L"%.1f Hz", fF0);
      lbl_f0_meas->SetText(arrBuf);
      UpdateError(fF0);
    }
    return true;
  }

  // VCC reading
  if(wcsstr(szLine, L"VCC:") && !wcsstr(szLine, L"VPEAL:")) {
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

// ────── ⋆⋅☆⋅⋆ ────────
// Helpers
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Reads sweep parameters from UI, validates them, clears the plot,
///   resets the progrss bar, and sends the SWEEP command
void LRCWindow::StartSweep() {
  wchar_t arrBuf[32];

  edit_fstart->GetText(arrBuf, 32);
  long lStart = wcstol(arrBuf, nullptr, 10);

  edit_fend->GetText(arrBuf, 32);
  long lEnd = wcstol(arrBuf, nullptr, 10);

  edit_steps->GetText(arrBuf, 32);
  int iSteps = (int)wcstol(arrBuf, nullptr, 10);

  if(lStart <- 0 || lEnd <= lStart || iSteps < 2 || iSteps > 500){
    MessageBox(hwnd_self,
               L"Invalid sweep parameters.\n\n"
               L"f start must be > 0\n"
               L"f end must be > f start\n"
               L"Steps must be 2-500",
              L"Sweep", MB_OK | MB_ICONWARNING);
    return;
  }

  // Build and send command: "SWEEP:lStart,lEnd,iSteps"
  char arrCmd[48];
  wsprintfA(arrCmd, "SWEEP:%ld,%ld,%d", lStart, lEnd, iSteps);
  port.Write(arrCmd);

  // Reset state
  plot.Clear();
  iExpectedSteps = iSteps;
  iReceivedSteps = 0;
  bSweepActive = true;

  prog_sweep->SetRange(0, 100);
  prog_sweep->SetPos(0);

  lbl_f0_meas->SetText(L"---");
  lbl_error->SetText(L"---");
  lbl_status->SetText(L"Starting sweep...");

  btn_sweep->Disable();
  btn_stop->Enable();
  btn_setvpin->Disable();
  btn_export->Disable();

  UpdateTheoreticalF0();
}

/// @brief Reads L and C from TextInput fields and calculates f0
///   Updates lbl_f0_theory. Called at startup and before each sweep
void LRCWindow::UpdateTheoreticalF0() {
  wchar_t arrBuf[32];

  edit_lval->GetText(arrBuf, 32);
  float fL_uH = wcstof(arrBuf, nullptr);

  edit_cval->GetText(arrBuf, 32);
  float fC_nF = wcstof(arrBuf, nullptr);

  if(fL_uH <= 0 || fC_nF <= 0) {
    lbl_f0_theory->SetText(L"---");
    return;
  }

  float fL_H = fL_uH * 1e-6f;
  float fC_F = fC_nF * 1e-9f;

  float fF0_theory = 1.0f / (2.0f * 3.14159265f * sqrtf(fL_H * fC_F));

  wsprintf(arrBuf, L"%.1f Hz", fF0_theory);
  lbl_f0_theory->SetText(arrBuf);
}

void LRCWindow::UpdateError(float fF0_measured) {
  wchar_t arrBuf[32];
  edit_lval->GetText(arrBuf, 32);
  float fL_uH = wcstof(arrBuf, nullptr);
  edit_cval->GetText(arrBuf, 32);
  float fC_nF = wcstof(arrBuf, nullptr);

  if(fL_uH <= 0 || fC_nF <= 0) return;

  float fL_H = fL_uH * 1e-6f;
  float fC_F = fC_nF * 1e-9f;
  float fF0_theory = 1.0f / (2.0f * 3.14159265f * sqrtf(fL_H * fC_F));
  float fErr_pct = fabsf(fF0_measured - fF0_theory) / fF0_theory * 100.0f;

  int iW = (int)fErr_pct;
  int iF = (int)((fErr_pct - iW) * 10);
  wsprintf(arrBuf, L"%d.%d%%", iW, iF);
  lbl_error->SetText(arrBuf);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnConnect / OnDisconnect
// ────── ⋆⋅☆⋅⋆ ────────
void LRCWindow::OnConnect() {
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

  btn_connect->Disable();
  btn_disc->Enable();
  btn_sweep->Enable();
  btn_setvpin->Enable();
  cmb_port->Disable();

  lbl_status->SetText(L"CONNECTED. Configure sweep and click Start.");
}

void LRCWindow::OnDisconnect() {
  KillTimer(hwnd_self, ID_TIMER_POLL);
  port.Close();
  bSweepActive = false;

  btn_connect->Enable();
  btn_disc->Disable();
  btn_sweep->Disable();
  btn_stop->Disable();
  btn_setvpin->Disable();
  cmb_port->Enable();

  lbl_status->SetText(L"Disconnected.");
  lbl_vcc->SetText(L"VCC: ---");

  ScanComPorts(cmb_port);
}