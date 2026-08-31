/// @file StrainGaugeWindow.cpp

#include "StrainGaugeWindow.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Export CSV
// ────── ⋆⋅☆⋅⋆ ────────
BOOL ExportCSV(const wchar_t* szPath, const StrainPlotPanel& plot, float fE_GPa) {
  HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hFile == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  DWORD dw = 0;
  const char* szHdr = "Time_ms,Microstrain_ue,Stress_MPa\r\n";
  WriteFile(hFile, szHdr, lstrlenA(szHdr), &dw, NULL);

  for(int i = 0; i < plot.iCount; i++) {
    float fUe = plot.arrStrain[i];
    float fStr_MPa = (fUe / 1000000.0f) * fE_GPa * 1000.0f;

    char arrRow[64];
    int iUe = (int)fUe;
    int iStr = (int)fStr_MPa;
    int iLen = wsprintfA(arrRow, "%d,%d,%d\r,\n", (int)plot.arrTime_ms[i], iUe, iStr);
    WriteFile(hFile, arrRow, iLen, &dw, NULL);
  }

  CloseHandle(hFile);
  return TRUE;
}

StrainGaugeWindow::StrainGaugeWindow()
  : fGain(10.0f)
  , fGF(2.0f)
  , fE_GPa(69.0f)
  , fH_mm(5.0f)
  , fL_mm(100.0f)
  , iADC_tare(512)
  , bTared(false)
  , fVcc(5.0f)
  , lRecStart(0)
  , cmb_port(nullptr)
  , btn_connect(nullptr)
  , btn_disc(nullptr)
  , btn_tare(nullptr)
  , btn_reset(nullptr)
  , edit_gain(nullptr)
  , edit_gf(nullptr)
  , edit_elastic(nullptr)
  , edit_h(nullptr)
  , edit_l(nullptr)
  , btn_apply(nullptr)
  , btn_export(nullptr)
  , lbl_adc_val(nullptr)
  , lbl_vadc_val(nullptr)
  , lbl_delta_val(nullptr)
  , lbl_strain_val(nullptr)
  , lbl_stress_val(nullptr)
  , lbl_defl_val(nullptr)
  , lbl_tare_status(nullptr)
  , lbl_vcc_val(nullptr)
{
  ZeroMemory(arrLineBuf, sizeof(arrLineBuf));
}

StrainGaugeWindow::~StrainGaugeWindow() {
  delete btn_connect;
  delete btn_disc;
  delete btn_tare;
  delete btn_reset;
  delete edit_gain;
  delete edit_gf;
  delete edit_elastic;
  delete edit_h;
  delete edit_l;
  delete btn_apply;
  delete btn_export;
  delete lbl_adc_val;
  delete lbl_vadc_val;
  delete lbl_delta_val;
  delete lbl_strain_val;
  delete lbl_stress_val;
  delete lbl_defl_val;
  delete lbl_tare_status;
  delete lbl_vcc_val;
}

void StrainGaugeWindow::OnCreate() {
  // Connection
  new Label(hwnd_self, L"COM Port:", 16, 18, 72, 24);
  cmb_port = new ComboBox(hwnd_self, ID_COMBO_PORT, 92, 14, 120, 200);
  btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, 220, 14, 90, 28);
  btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, 318, 14, 100, 28);
  btn_disc->Disable();
  ScanComPorts(cmb_port);
  // Tare controls
  btn_tare = new Button(hwnd_self, L"Tare (zero)", ID_BTN_TARE, 16, 54, 110, 28);
  btn_reset = new Button(hwnd_self, L"Reset tare", ID_BTN_RESET, 134, 54, 100, 28);
  lbl_tare_status = new Label(hwnd_self, L"Status: Not tared", 242, 60, 260, 22);
  btn_tare->Disable();
  btn_reset->Disable();
  // Sensor parameters
  new Label(hwnd_self, L"Gain:", 112, 102, 28, 22);
  edit_gf = new TextInput(hwnd_self, -1, 144, 96, 44, 28);
  edit_gf->SetText(L"2.0");
  new Label(hwnd_self, L"GF:", 112, 102, 28, 22);
  edit_gf = new TextInput(hwnd_self, -1, 144, 96, 44, 28);
  edit_gf->SetText(L"2.0");
  new Label(hwnd_self, L"E (GPa):", 196, 102, 62, 22);
  edit_elastic = new TextInput(hwnd_self, -1, 262, 96, 44, 28);
  edit_elastic->SetText(L"69");
  new Label(hwnd_self, L"h (mm):", 314, 102, 56, 22);
  edit_h = new TextInput(hwnd_self, -1, 374, 96, 44, 28);
  edit_h->SetText(L"5");
  new Label(hwnd_self, L"L (mm):", 426, 102, 56, 22);
  edit_l = new TextInput(hwnd_self, -1, 486, 96, 44, 28);
  edit_l->SetText(L"100");
  btn_apply  = new Button(hwnd_self, L"Apply", ID_BTN_APPLY, 538, 96, 60, 28);
  btn_export = new Button(hwnd_self, L"Export CSV", ID_BTN_EXPORT, 606, 96, 90, 28);
  btn_export->Disable();
  // Readouts 
  new Label(hwnd_self, L"ADC avg:", 16, 140, 90, 22);
  new Label(hwnd_self, L"V_adc:", 16, 164, 90, 22);
  new Label(hwnd_self, L"ΔADC:", 16, 188, 90, 22);
  new Label(hwnd_self, L"Strain:", 16, 212, 90, 22);
  new Label(hwnd_self, L"Stress:", 200, 212, 90, 22);
  new Label(hwnd_self, L"δ (est):", 384, 212, 90, 22);
  new Label(hwnd_self, L"VCC:", 500, 140, 40, 22);
  lbl_adc_val = new Label(hwnd_self, L"---", 110, 140, 80, 22);
  lbl_vadc_val = new Label(hwnd_self, L"---", 110, 164, 100, 22);
  lbl_delta_val = new Label(hwnd_self, L"---", 110, 188, 80, 22);
  lbl_strain_val = new Label(hwnd_self, L"---", 110, 212, 80, 22);
  lbl_stress_val = new Label(hwnd_self, L"---", 294, 212, 80, 22);
  lbl_defl_val = new Label(hwnd_self, L"---", 478, 212, 80, 22);
  lbl_vcc_val = new Label(hwnd_self, L"---", 544, 140, 90, 22);
  // Strain history plot 
  new Label(hwnd_self, L"Strain history  (green = tension, red = compression):", 16, 242, 450, 18);
  plot.Init(16, 264, 688, 280);
}

void StrainGaugeWindow::OnPaint(HDC hdc) {
  plot.Draw(hdc);
}

void StrainGaugeWindow::OnCommand(int iControlId, int iNotifCode) {
  switch (iControlId) {
    case ID_BTN_CONNECT:
      OnConnect();
      break;
    case ID_BTN_DISCONNECT:
      OnDisconnect();
      break;
    case ID_BTN_TARE:
      port.Write("TARE");
      lbl_tare_status->SetText(L"Status: Taring...");
      break;
    case ID_BTN_RESET:
      port.Write("RESET");
      iADC_tare = 512;
      bTared = false;
      lbl_tare_status->SetText(L"Status: Not tared");
      lbl_strain_val->SetText(L"---");
      lbl_stress_val->SetText(L"---");
      lbl_defl_val->SetText(L"---");
      break;
    case ID_BTN_APPLY:
      ApplySettings();
      break;
    case ID_BTN_EXPORT: {
      if(plot.iCount == 0) {
        MessageBox(hwnd_self, L"No data to export.", L"Export", MB_OK | MB_ICONWARNING);
        return;
      }
      wchar_t arrPath[MAX_PATH];
      if(!ShowSaveDialog(hwnd_self, L"CSV Files\0*.csv\0All Files\0*.*\0", L"csv", arrPath, MAX_PATH)) {
        return;
      }
      if(!ExportCSV(arrPath, plot, fE_GPa)) {
        MessageBox(hwnd_self, L"Export failed.", L"Export", MB_OK | MB_ICONERROR);
      }
      break;
    }
  }
}

void StrainGaugeWindow::OnTimer(int iTimerId) {
  if(iTimerId != ID_TIMER_POLL) {
    return;
  }

  if(!port.IsOpen()) {
    return;
  }

  char arrRaw[READ_BUF];
  DWORD dwRead = 0;
  port.Read(arrRaw, READ_BUF, dwRead);
  if(dwRead == 0) {
    return;
  }

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
  if(iRemaining > 0) {
    memmove(arrLineBuf, pLine, iRemaining);
  }

  iLineBufLen = iRemaining;
  arrLineBuf[iLineBufLen] = '\0';

  if(bRepaint) {
    InvalidateRect(hwnd_self, NULL, FALSE);
  }
}

void StrainGaugeWindow::OnDestroy() {
  KillTimer(hwnd_self, ID_TIMER_POLL);
  port.Close();
}

bool StrainGaugeWindow::ParseLine(const wchar_t* szLine) {
  
}