/// @file ACMotorApp.cpp

#include "ACMotorApp.h"
#include <cwchar> // swprintf

ACMotorApp::ACMotorApp()
  : lbl_V(nullptr)
  , txti_V(nullptr)
  , lbl_Freq(nullptr)
  , txti_Freq(nullptr)
  , lbl_Poles(nullptr)
  , txti_Poles(nullptr)
  , lbl_R2(nullptr)
  , txti_R2(nullptr)
  , lbl_X2(nullptr)
  , txti_X2(nullptr)
  , btn_seq(nullptr)
  , lbl_out_Ns(nullptr)
  , lbl_out_Nr(nullptr)
  , lbl_out_slip(nullptr)
  , lbl_out_Tbreak(nullptr)
  , lbl_out_Tstart(nullptr)
  , lbl_out_sbreak(nullptr)
  , lbl_status(nullptr)
  , m_inputs({})
  , m_state({})
  , m_fTheta_rad(0.0f)
  , m_fAnimDir(1.0f)
{}

ACMotorApp::~ACMotorApp() {
  delete lbl_V;
  delete txti_V;
  delete lbl_Freq;
  delete txti_Freq;
  delete lbl_Poles;
  delete txti_Poles;
  delete lbl_R2;
  delete txti_R2;
  delete lbl_X2;
  delete txti_X2;
  delete btn_seq;
  delete lbl_out_Ns;
  delete lbl_out_Nr;
  delete lbl_out_slip;
  delete lbl_out_Tbreak;
  delete lbl_out_Tstart;
  delete lbl_out_sbreak;
  delete lbl_status;
}

void ACMotorApp::OnCreate() {
  HWND hwnd = hwnd_self;

  // Input controls
  lbl_V = new Label(hwnd, L"V (Vrms):", 0, 0, 70, 18);
  txti_V = new TextInput(hwnd, ID_INPUT_V, 0, 0, 70, 22);

  lbl_Freq = new Label(hwnd, L"f (Hz):", 0, 0, 55, 18);
  txti_Freq = new TextInput(hwnd, ID_INPUT_FREQ,  0, 0, 70, 22);

  lbl_Poles = new Label(hwnd, L"Poles:", 0, 0, 45, 18);
  txti_Poles = new TextInput(hwnd, ID_INPUT_POLES, 0, 0, 40, 22);

  lbl_R2 = new Label(hwnd, L"R2 (Ω):", 0, 0, 55, 18);
  txti_R2 = new TextInput(hwnd, ID_INPUT_R2,    0, 0, 70, 22);

  lbl_X2 = new Label(hwnd, L"X2 (Ω):", 0, 0, 55, 18);
  txti_X2 = new TextInput(hwnd, ID_INPUT_X2, 0, 0, 70, 22);

  // Sequence toggle
  btn_seq = new Button(hwnd, L"A-B-C", ID_BTN_SEQ, 0, 0, 90, 26);

  // Readout labels
  lbl_out_Ns = new Label(hwnd, L"Ns: ---", 0, 0, 160, 18);
  lbl_out_Nr = new Label(hwnd, L"Nr: ---", 0, 0, 160, 18);
  lbl_out_slip = new Label(hwnd, L"Slip: ---", 0, 0, 130, 18);
  lbl_out_Tbreak = new Label(hwnd, L"T_bk: ---", 0, 0, 160, 18);
  lbl_out_Tstart = new Label(hwnd, L"T_st: ---", 0, 0, 160, 18);
  lbl_out_sbreak = new Label(hwnd, L"s_bk: ---", 0, 0, 130, 18);
  lbl_status = new Label(hwnd, L"", 0, 0, 120, 18);

  // Defaults
  m_inputs.iPoles = 4; // iPoles must be set before EN_CHANGE fires
  txti_V->SetText(L"230");
  txti_Freq->SetText(L"60");
  txti_Poles->SetText(L"4");
  txti_R2->SetText(L"1");
  txti_X2->SetText(L"3");

  // Layout before first paint
  RECT rc;
  GetClientRect(hwnd, &rc);
  LayoutControls(rc.right, rc.bottom);

  // Start animation timer. Fires every 16ms regardless of input state
  // The field panel draws a place holder if m_state.bValid is false
  SetTimer(hwnd_self, ID_TIMER_ANIM, TIMER_MS, NULL);
}

void ACMotorApp::OnDestroy() {
  KillTimer(hwnd_self, ID_TIMER_ANIM);
}

void ACMotorApp::OnCommand(int iControlId, int iNotifCode) {
  if(iNotifCode == EN_CHANGE) {
    switch(iControlId) {
      case ID_INPUT_V:
      case ID_INPUT_FREQ:
      case ID_INPUT_POLES:
      case ID_INPUT_R2:
      case ID_INPUT_X2:
        RecalcAndRedraw();
        break;
    }
    return;
  }

  if(iNotifCode == BN_CLICKED && iControlId == ID_BTN_SEQ) {
    if(m_fAnimDir > 0.0f) {
      // ABC -> ACB
      m_fAnimDir = -1.0f;
      btn_seq->SetText(L"A-C-B");
    } else {
      // ACB -> ABC
      m_fAnimDir = 1.0f;
      btn_seq->SetText(L"A-B-C");
    }
  }
}

void ACMotorApp::OnPaint(HDC hdc) {
  RECT rcClient;
  GetClientRect(hwnd_self, &rcClient);
  int iW = rcClient.right;
  int iH = rcClient.bottom;

  // FillRect(hdc, &rcClient, GetSysColorBrush(COLOR_WINDOW));

  RECT rcControls = { 0, 0, iW, CONTROLS_H };
  FillRect(hdc, &rcControls, GetSysColorBrush(COLOR_WINDOW));

  RECT rcReadout = { 0, iH - READOUT_H, iW, iH };
  FillRect(hdc, &rcReadout, GetSysColorBrush(COLOR_WINDOW));

  // Separator lines
  HPEN hSepPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
  HPEN hOldPen = (HPEN)SelectObject(hdc, hSepPen);

  MoveToEx(hdc, 0, CONTROLS_H, NULL);
  LineTo(hdc, iW, CONTROLS_H);

  MoveToEx(hdc, 0, iH - READOUT_H, NULL);
  LineTo(hdc, iW, iH - READOUT_H);

  int iSplit = (iW * 3) / 5;
  MoveToEx(hdc, iSplit, CONTROLS_H, NULL);
  LineTo(hdc, iSplit, iH - READOUT_H);

  SelectObject(hdc, hOldPen);
  DeleteObject(hSepPen);

  // GDI Panels
  m_torquePanel.Draw(hdc, GetTorquePanelRect(), m_state);
  m_fieldPanel.Draw(hdc, GetFieldPanelRect(), m_state, m_fTheta_rad, m_fAnimDir > 0.0f);
}

void ACMotorApp::OnTimer(int iTimerId) {
  if(iTimerId != ID_TIMER_ANIM) {
    return;
  }

  // Advance the field angle by one timer step.
  // fOmega is in rad/s. Multiply by elapsed seconds to get radians.
  // m_fAnimDir flips sign for ACB sequence
  if(m_state.bValid) {
    const float fDt = TIMER_MS / 1000.0f; // seconds per tick
    m_fTheta_rad += m_state.fOmega_s * fDt * m_fAnimDir;

    // Wrap into [0, 2pi]
    const float fTwoPi = 2.0f * M_PI;
    while (m_fTheta_rad >= fTwoPi) {
      m_fTheta_rad -= fTwoPi;
    }
    while (m_fTheta_rad < 0.0f) {
      m_fTheta_rad += fTwoPi;
    }
  }

  // Only invalidate the field panel. The torque curce is static
  //   between input changes and does not need to repaint every tick
  RECT rcField = GetFieldPanelRect();
  InvalidateRect(hwnd_self, &rcField, FALSE);
}

LRESULT ACMotorApp::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if(uMsg == WM_ERASEBKGND) { // Prevent field Panel from flickering
    return 1;
  }
  
  if(uMsg == WM_SIZE) {
    LayoutControls(LOWORD(lParam), HIWORD(lParam));
    InvalidateRect(hwnd_self, NULL, TRUE);
    return 0;
  }
  return Window::OnMessage(uMsg, wParam, lParam);
}

void ACMotorApp::LayoutControls(int iW, int iH) {
  const int iRowH = 22;
  const int iLblH = 18;
  const int iGap = 10;

  // Label + input widths per group
  const int iGW_V = 70 + 4 + 70; // V
  const int iGW_F = 55 + 4 + 70; // f
  const int iGW_P = 45 + 4 + 40; // Poles
  const int iGW_R = 55 + 4 + 70; // R2
  const int iGW_X = 55 + 4 + 70; // X2

  const int iTotalW = iGW_V + iGW_F + iGW_P + iGW_R + iGW_X + 4 * iGap;
  const int iRow1Y  = MARGIN;
  int iX = (iW - iTotalW) / 2;

  // Row 1: all five inputs centred
  MoveWindow(lbl_V->GetHandle(), iX, iRow1Y + 2, 70, iLblH, TRUE);
  MoveWindow(txti_V->GetHandle(), iX + 74, iRow1Y, 70, iRowH, TRUE);
  iX += iGW_V + iGap;

  MoveWindow(lbl_Freq->GetHandle(), iX, iRow1Y + 2, 55, iLblH, TRUE);
  MoveWindow(txti_Freq->GetHandle(), iX + 59, iRow1Y, 70, iRowH, TRUE);
  iX += iGW_F + iGap;

  MoveWindow(lbl_Poles->GetHandle(), iX, iRow1Y + 2, 45, iLblH, TRUE);
  MoveWindow(txti_Poles->GetHandle(), iX + 49, iRow1Y, 40, iRowH, TRUE);
  iX += iGW_P + iGap;

  MoveWindow(lbl_R2->GetHandle(), iX, iRow1Y + 2, 55, iLblH, TRUE);
  MoveWindow(txti_R2->GetHandle(), iX + 59, iRow1Y, 70, iRowH, TRUE);
  iX += iGW_R + iGap;

  MoveWindow(lbl_X2->GetHandle(), iX, iRow1Y + 2, 55, iLblH, TRUE);
  MoveWindow(txti_X2->GetHandle(), iX + 59, iRow1Y, 70, iRowH, TRUE);

  // Row 2: sequence toggle centred
  const int iBtnW = 90;
  const int iBtnH = 26;
  const int iRow2Y = MARGIN + 32;
  MoveWindow(btn_seq->GetHandle(), (iW - iBtnW) / 2, iRow2Y, iBtnW, iBtnH, TRUE);

  // Readout strip — 7 equal columns
  const int iColW = iW / 7;
  const int iOutH = 18;
  const int iRdY  = iH - READOUT_H + (READOUT_H - iOutH) / 2;
  const int iOutW = iColW - MARGIN;

  MoveWindow(lbl_out_Ns->GetHandle(), 0, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_Nr->GetHandle(), iColW, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_slip->GetHandle(), iColW * 2, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_Tbreak->GetHandle(), iColW * 3, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_Tstart->GetHandle(), iColW * 4, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_sbreak->GetHandle(), iColW * 5, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_status->GetHandle(), iColW * 6, iRdY, iOutW, iOutH, TRUE);
}

RECT ACMotorApp::GetTorquePanelRect() const {
  RECT rc;
  GetClientRect(hwnd_self, &rc);
  int iSplit = (rc.right * 3) / 5;
  return { 0, CONTROLS_H + 1, iSplit, rc.bottom - READOUT_H };
}

RECT ACMotorApp::GetFieldPanelRect() const {
  RECT rc;
  GetClientRect(hwnd_self, &rc);
  int iSplit = (rc.right * 3) / 5;
  return { iSplit + 1, CONTROLS_H + 1, rc.right, rc.bottom - READOUT_H };
}

void ACMotorApp::RecalcAndRedraw() {
  m_inputs.fV_rms = ReadInputFloat(txti_V, 0.0f);
  m_inputs.fFreq_Hz = ReadInputFloat(txti_Freq, 0.0f);
  m_inputs.iPoles = ReadInputInt(txti_Poles, 0);
  m_inputs.fR2 = ReadInputFloat(txti_R2, 0.0f);
  m_inputs.fX2 = ReadInputFloat(txti_X2, 0.0f);

  m_state = Compute(m_inputs);

  UpdateReadouts();

  // TorquePanel: Full redraw on input change
  // Field panel: timer drives it. No explicit invalidate needed here
  RECT rcTorque = GetTorquePanelRect();
  InvalidateRect(hwnd_self, &rcTorque, TRUE);
}

void ACMotorApp::UpdateReadouts() {
  if(!m_state.bValid) {
    lbl_out_Ns->SetText(L"Ns: ---");
    lbl_out_Nr->SetText(L"Nr: ---");
    lbl_out_slip->SetText(L"Slip: ---");
    lbl_out_Tbreak->SetText(L"T_bk: ---");
    lbl_out_Tstart->SetText(L"T_st: ---");
    lbl_out_sbreak->SetText(L"s_bk: ---");
    lbl_status->SetText(L"⚠ Invalid");
    return;
  }

  lbl_status->SetText(L"");
  
  wchar_t arrBuf[64];
  
  swprintf(arrBuf, 64, L"Ns: %.0f RPM", m_state.fNs_rpm);
  lbl_out_Ns->SetText(arrBuf);
  
  swprintf(arrBuf, 64, L"Nr: %.0f RPM", m_state.fNr_fullload);
  lbl_out_Nr->SetText(arrBuf);
  
  swprintf(arrBuf, 64, L"Slip: %.1f%%", FULLLOAD_SLIP * 100.0f);
  lbl_out_slip->SetText(arrBuf);
  
  // Normalized torques displayed as ratios relative to breakdown peak
  swprintf(arrBuf, 64, L"T_bk: 1.00");
  lbl_out_Tbreak->SetText(arrBuf);
  
  swprintf(arrBuf, 64, L"T_st: %.2f", m_state.fT_start);
  lbl_out_Tstart->SetText(arrBuf);
  
  swprintf(arrBuf, 64, L"s_bk: %.3f", m_state.fSlip_breakdown);
  lbl_out_sbreak->SetText(arrBuf);
}

float ACMotorApp::ReadInputFloat(TextInput* pInput, float fDefault) const {
  wchar_t arrBuf[32];
  pInput->GetText(arrBuf, 32);
  if(arrBuf[0] == L'\0') {
    return fDefault;
  }
  wchar_t* pEnd = nullptr;
  float fVal = wcstof(arrBuf, &pEnd);
  return (pEnd == arrBuf) ? fDefault : fVal;
}

int ACMotorApp::ReadInputInt(TextInput* pInput, int iDefault) const {
  wchar_t arrBuf[16];
  pInput->GetText(arrBuf, 16);
  if(arrBuf[0] == L'\0') {
    return iDefault;
  }
  wchar_t* pEnd = nullptr;
  long lVal = wcstol(arrBuf, &pEnd, 10);
  return (pEnd == arrBuf) ? iDefault : (int)lVal;
}