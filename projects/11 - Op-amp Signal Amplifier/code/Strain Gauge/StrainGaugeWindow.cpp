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
  // Telemetry: "SAVG:xxx,VADC:x.xxx,DELTA:xxxx,TARED:n"
  if(wcsstr(szLine, L"SAVEG:") && wcsstr(szLine, L"DELTA:")) {
    float fSavg = ParseFloat(szLine, L"SAVG:");
    float fVadc = ParseFloat(szLine, L"VADC:");
    float fDelta = ParseFloat(szLine, L"DELTA:");
    float fTared = ParseFloat(szLine, L"TARED:");

    if (fSavg < 0 || fVadc < 0) {
      return false;
    }
    bTared = (fTared > 0.5f);

    // Raw readouts
    wchar_t arrBuf[32];
    wsprintf(arrBuf, L"%d", (int)fSavg);
    lbl_adc_val->SetText(arrBuf);

    int iVW = (int)fVadc;
    int iVF = (int)((fVadc - iVW) * 1000);
    wsprintf(arrBuf, L"%d.%03d V", iVW, iVF);
    lbl_vadc_val->SetText(arrBuf);

    wsprintf(arrBuf, L"%+d ADC", (int)fDelta);
    lbl_delta_val->SetText(arrBuf);

    // Compute µε and stress if tared
    if (bTared) {
      UpdateReadouts((int)fDelta, fVadc);

      long lNow = (long)GetTickCount();
      if (lRecStart == 0) {
        lRecStart = lNow;
      }

      // Compute µε for plot
      float fUe = (4.0f * fDelta * 1000000.0f) / (1023.0f * fGain * fGF);
      plot.AddSample((float)(lNow - lRecStart), fUe);
      btn_export->Enable();
    } else {
      lbl_strain_val->SetText(L"--- (not tared)");
      lbl_stress_val->SetText(L"---");
      lbl_defl_val->SetText(L"---");
    }

    return true;
  }

  // TARE acknowledgement
  if (wcsstr(szLine, L"TARE:OK")) {
    float fAdc = ParseFloat(szLine, L"ADC:");
    float fV   = ParseFloat(szLine, L"VCC:");

    if (fAdc >= 0) {
      iADC_tare = (int)fAdc;
    }
    if (fV   >  0) {
      fVcc = fV;
    }

    bTared = true;

    wchar_t arrBuf[48];
    wsprintf(arrBuf, L"Status: Tared  (ADC = %d)", iADC_tare);
    lbl_tare_status->SetText(arrBuf);

    // Reset plot on tare — new baseline
    plot.Clear();
    lRecStart = 0;
    return false;
  }

  // VCC reading
  if(wcsstr(szLine, L"VCC:") && !wcsstr(szLine, L"TARE:")) {
    float fV = ParseFloat(szLine, L"VCC:");
    if(fV > 0.0f) {
      fVcc = fV;
      wchar_t arrBuf[16];
      int iW = (int)fV;
      int iF = (int)((fV - iW) * 1000);
      wsprintf(arrBuf, L"%d.%03d V", iW, iF);
      lbl_vcc_val->SetText(arrBuf);
    }
    return false;
  }

  // RESET acknowledgement
  if(wcsstr(szLine, L"RESET:OK")) {
    plot.Clear();
    lRecStart = 0;
    return true;
  }
  return false;
}

void StrainGaugeWindow::UpdateReadouts(int iDelta, float fVadc) {
  // Microstrain 
  // ε = 4 × ΔADC / (1023 × Gain × GF)
  // ε_µ = ε × 1,000,000
  float fUe = (4.0f * iDelta * 1000000.0f) / (1023.0f * fGain * fGF);

  wchar_t arrBuf[32];
  wsprintf(arrBuf, L"%+d µε", (int)fUe);
  lbl_strain_val->SetText(arrBuf);

  // Stress (MPa)
  // σ = E × ε   (E in Pa = fE_GPa × 10⁹)
  float fStress_MPa = (fUe / 1000000.0f) * (fE_GPa * 1000.0f);
  int   iStW = (int)fStress_MPa;
  int   iStF = abs((int)((fStress_MPa - (int)fStress_MPa) * 10));
  wsprintf(arrBuf, L"%+d.%01d MPa", iStW, iStF);
  lbl_stress_val->SetText(arrBuf);

  // Estimated tip deflection (mm)
  // From cantilever bending: δ = ε × L² / (3 × h/2)
  // where ε is dimensionless strain, L and h are in consistent units
  if (fH_mm > 0.001f && fL_mm > 0.001f) {
    float fL_m = fL_mm / 1000.0f;
    float fH_m = fH_mm / 1000.0f;
    float fEps = fUe / 1000000.0f;
    float fDef_m = (fEps * fL_m * fL_m) / (3.0f * (fH_m / 2.0f));
    int iDef_mm  = (int)(fDef_m * 1000.0f);
    int iDef_frac = abs((int)((fDef_m * 1000.0f - (int)(fDef_m * 1000.0f)) * 10));
    wsprintf(arrBuf, L"%+d.%01d mm", iDef_mm, iDef_frac);
    lbl_defl_val->SetText(arrBuf);
  }
}

void StrainGaugeWindow::ApplySettings() {
  wchar_t arrBuf[32];

  edit_gain->GetText(arrBuf, 32);    
  fGain = wcstof(arrBuf, nullptr);

  edit_gf->GetText(arrBuf, 32);      
  fGF = wcstof(arrBuf, nullptr);

  edit_elastic->GetText(arrBuf, 32); 
  fE_GPa = wcstof(arrBuf, nullptr);

  edit_h->GetText(arrBuf, 32);       
  fH_mm = wcstof(arrBuf, nullptr);

  edit_l->GetText(arrBuf, 32);       
  fL_mm = wcstof(arrBuf, nullptr);

  if(fGain < 1.0f) { 
    fGain = 1.0f;  
    edit_gain->SetText(L"1"); 
  }
  if(fGF < 0.5f) { 
    fGF = 2.0f;  
    edit_gf->SetText(L"2.0"); 
  }
  if(fE_GPa < 1.0f) { 
    fE_GPa = 69.0f; 
    edit_elastic->SetText(L"69"); 
  }
  if(fH_mm  < 0.1f) { 
    fH_mm  = 5.0f;  
    edit_h->SetText(L"5"); 
  }
  if(fL_mm  < 1.0f) { 
    fL_mm  = 100.0f;
    edit_l->SetText(L"100"); 
  }
}

void StrainGaugeWindow::OnConnect() {
  if(cmb_port->GetCount() == 0) {
    MessageBox(hwnd_self, L"No COM ports found.", L"Connect", MB_OK | MB_ICONWARNING);
    return;
  }

  wchar_t arrPortName[16];
  cmb_port->GetSelected(arrPortName, 16);

  // 9600 baud
  if(!port.Open(arrPortName, 9600)) {
    wchar_t arrMsg[64];
    wsprintf(arrMsg, L"Failed to open %s.\nError: %lu", arrPortName, port.GetLastErrorCode());
    MessageBox(hwnd_self, arrMsg, L"Connect", MB_OK | MB_ICONERROR);
    return;
  }

  SetTimer(hwnd_self, ID_TIMER_POLL, POLL_MS, NULL);
  lRecStart = 0;

  btn_connect->Disable();
  btn_disc->Enable();
  btn_tare->Enable();
  btn_reset->Enable();
  cmb_port->Disable();
}

void StrainGaugeWindow::OnDisconnect() {
  KillTimer(hwnd_self, ID_TIMER_POLL);
  port.Close();

  btn_connect->Enable();
  btn_disc->Disable();
  btn_tare->Disable();
  btn_reset->Disable();
  btn_export->Disable();
  cmb_port->Enable();

  lbl_adc_val->SetText(L"---");
  lbl_vadc_val->SetText(L"---");
  lbl_delta_val->SetText(L"---");
  lbl_strain_val->SetText(L"---");
  lbl_stress_val->SetText(L"---");
  lbl_defl_val->SetText(L"---");
  lbl_vcc_val->SetText(L"---");

  ScanComPorts(cmb_port);
}