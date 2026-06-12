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

  // Calculate and display theoretical f_0, from default L/C values
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