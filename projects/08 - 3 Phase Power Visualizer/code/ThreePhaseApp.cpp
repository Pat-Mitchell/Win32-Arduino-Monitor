/// @file ThreePhaseApp.cpp

#include "ThreePhaseApp.h"
#include <cwchar> // swprintf

// ────── ⋆⋅☆⋅⋆ ────────
// Constructors/Destructors
// ────── ⋆⋅☆⋅⋆ ────────

ThreePhaseApp::ThreePhaseApp()
  : lbl_V(nullptr)
  , txti_V(nullptr)
  , lbl_Freq(nullptr)
  , txti_Freq(nullptr)
  , lbl_R(nullptr)
  , txti_R(nullptr)
  , btn_config(nullptr)
  , btn_seq(nullptr)
  , lbl_out_Vph(nullptr)
  , lbl_out_VL(nullptr)
  , lbl_out_Iph(nullptr)
  , lbl_out_IL(nullptr)
  , lbl_out_P(nullptr)
  , lbl_out_neutral(nullptr)
  , lbl_out_config(nullptr)
  , lbl_status(nullptr)
  , m_inputs({})
  , m_state({})
  {}

ThreePhaseApp::~ThreePhaseApp() {
  delete lbl_V;
  delete txti_V;
  delete lbl_Freq;
  delete txti_Freq;
  delete lbl_R;
  delete txti_R;
  delete btn_config;
  delete btn_seq;
  delete lbl_out_Vph;
  delete lbl_out_VL;
  delete lbl_out_Iph;
  delete lbl_out_IL;
  delete lbl_out_P;
  delete lbl_out_neutral;
  delete lbl_out_config;
  delete lbl_status;
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnCreate
// ────── ⋆⋅☆⋅⋆ ────────

void ThreePhaseApp::OnCreate() {
  HWND hwnd = hwnd_self;

  // Input controls
  lbl_V = new Label(hwnd, L"V (Vrms):", 0, 0, 70, 18);
  txti_V = new TextInput(hwnd, ID_INPUT_V, 0, 0, 75, 22);

  lbl_Freq = new Label(hwnd, L"f (Hz):", 0,0, 55, 18);
  txti_Freq = new TextInput(hwnd, ID_INPUT_FREQ, 0, 0, 75, 22);

  lbl_R = new Label(hwnd, L"R (Ω):", 0, 0, 55, 18);
  txti_R = new TextInput(hwnd, ID_INPUT_R, 0, 0, 75, 22);

  // Toggle buttons. Labels set to default state (star, abc)
  btn_config = new Button(hwnd, L"Star (Y)", ID_BTN_CONFIG, 0, 0, 100, 26);
  btn_seq = new Button(hwnd, L"A-B-C", ID_BTN_SEQ, 0, 0, 90, 26);

  // Readout labels
  lbl_out_Vph = new Label(hwnd, L"Vph: ---", 0, 0, 160, 18);
  lbl_out_VL = new Label(hwnd, L"VL: ---", 0, 0, 160, 18);
  lbl_out_Iph = new Label(hwnd, L"Iph: ---", 0, 0, 160, 18);
  lbl_out_IL = new Label(hwnd, L"IL: ---", 0, 0, 160, 18);
  lbl_out_P = new Label(hwnd, L"P: ---", 0, 0, 160, 18);
  lbl_out_neutral = new Label(hwnd, L"IN: ---", 0, 0, 160, 18);
  lbl_out_config = new Label(hwnd, L"", 0, 0, 220, 18);
  lbl_status = new Label(hwnd, L"", 0, 0, 120, 18);

  // ────── ⋆⋅☆⋅⋆ ────────
  // Defaults
  // ────── ⋆⋅☆⋅⋆ ────────
  // 120Vrms, 60Hz, 100 Ohms, Star, ABC
  // ────── ⋆⋅☆⋅⋆ ────────

  m_inputs.eConfig = Config::Star;
  m_inputs.eSeq = PhaseSeq::ABC;

  txti_V->SetText(L"120");
  txti_Freq->SetText(L"60");
  txti_R->SetText(L"100");

  RECT rc;
  GetClientRect(hwnd, &rc);
  LayoutControls(rc.right, rc.bottom);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnCommand
// ────── ⋆⋅☆⋅⋆ ────────

void ThreePhaseApp::OnCommand(int iControlId, int iNotifCode) {
  // EN_CHANGE fires on every keystroke
  if(iNotifCode == EN_CHANGE) {
    switch(iControlId) {
      case ID_INPUT_V:
      case ID_INPUT_FREQ:
      case ID_INPUT_R:
        RecalcAndRedraw();
        break;
    }
  }

  // BN_CLICKED toggle buttons filp their eunm and update their label
  if(iNotifCode == BN_CLICKED) {
    if(iControlId == ID_BTN_CONFIG) {
      if(m_inputs.eConfig == Config::Star) {
        m_inputs.eConfig = Config::Delta;
        btn_config->SetText(L"Delta (Δ)");
      } else {
        m_inputs.eConfig = Config::Star;
        btn_config->SetText(L"Star (Y)");
      }
      RecalcAndRedraw();
    } else if(iControlId == ID_BTN_SEQ) {
      if(m_inputs.eSeq == PhaseSeq::ABC) {
        m_inputs.eSeq = PhaseSeq::ACB;
        btn_seq->SetText(L"A-C-B");
      } else {
        m_inputs.eSeq = PhaseSeq::ABC;
        btn_seq->SetText(L"A-B-C");
      }
      RecalcAndRedraw();
    }
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnPaint
// ────── ⋆⋅☆⋅⋆ ────────

void ThreePhaseApp::OnPaint(HDC hdc) {
  RECT rcClient;
  GetClientRect(hwnd_self, &rcClient);
  int iW = rcClient.right;
  int iH = rcClient.bottom;

  FillRect(hdc, &rcClient, GetSysColorBrush(COLOR_WINDOW));

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

  // GDI panel
  m_waveformPanel.Draw(hdc, GetWaveformPanelRect(), m_state);
  m_phasorPanel.Draw(hdc, GetPhasorPanelRect(), m_state);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnMessage
// catch MW_SIZE
// ────── ⋆⋅☆⋅⋆ ────────

LRESULT ThreePhaseApp::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if(uMsg == WM_SIZE) {
    LayoutControls(LOWORD(lParam), HIWORD(lParam));
    InvalidateRect(hwnd_self, NULL, TRUE);
    return 0;
  }
  return Window::OnMessage(uMsg, wParam, lParam);
}

// Layout Controls
void ThreePhaseApp::LayoutControls(int iW, int iH) {
  const int iLblH = 18;
  const int iInputW = 75;
  const int iRowH = 22;
  const int iGap = 10;
  const int iLblW_V = 70;
  const int iLblW_FR = 55;

  // Row 1: V, f, R
  const int iGroupW_V = iLblW_V + 4 + iInputW;
  const int iGroupW_FR = iLblW_FR + 4 + iInputW;
  const int iTotalW1 = iGroupW_V + iGroupW_FR + iGroupW_FR + 2 * iGap;
  const int iRow1Y = MARGIN;
  int iX = (iW - iTotalW1) / 2;

  MoveWindow(lbl_V->GetHandle(), iX, iRow1Y + 2, iLblW_V, iLblH, TRUE);
  MoveWindow(txti_V->GetHandle(), iX + iLblW_V + 4, iRow1Y, iInputW, iRowH, TRUE);
  iX += iGroupW_V + iGap;

  MoveWindow(lbl_Freq->GetHandle(), iX, iRow1Y + 2, iLblW_FR, iLblH, TRUE);
  MoveWindow(txti_Freq->GetHandle(), iX + iLblW_FR + 4, iRow1Y, iInputW, iRowH, TRUE);
  iX += iGroupW_FR + iGap;

  MoveWindow(lbl_R->GetHandle(), iX, iRow1Y + 2, iLblW_FR, iLblH, TRUE);
  MoveWindow(txti_R->GetHandle(), iX + iLblW_FR + 4, iRow1Y, iInputW, iRowH, TRUE);

  // Row 2: Config toggle, seq toggle
  const int iBtnW_Config = 100;
  const int iBtnW_Seq = 90;
  const int iBtnH = 26;
  const int iBtnGap = 20;
  const int iTotalW2 = iBtnW_Config + iBtnGap + iBtnW_Seq;
  const int iRow2Y = MARGIN + 30;

  iX = (iW - iTotalW2) / 2;
  MoveWindow(btn_config->GetHandle(), iX, iRow2Y, iBtnW_Config, iBtnH, TRUE);
  MoveWindow(btn_seq->GetHandle(), iX + iBtnW_Config + iBtnGap, iRow2Y, iBtnW_Seq, iBtnH, TRUE);

  // Readout strip
  const int iColW = iW / 8;
  const int iOutH = 18;
  const int iRdY  = iH - READOUT_H + (READOUT_H - iOutH) / 2;
  const int iOutW = iColW - MARGIN;

  MoveWindow(lbl_out_Vph->GetHandle(), 0, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_VL->GetHandle(), iColW, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_Iph->GetHandle(), iColW * 2, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_IL->GetHandle(), iColW * 3, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_P->GetHandle(), iColW * 4, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_neutral->GetHandle(), iColW * 5, iRdY, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_config->GetHandle(), iColW * 6, iRdY, iOutW * 2, iOutH, TRUE);
  MoveWindow(lbl_status->GetHandle(), iColW * 7, iRdY, iOutW, iOutH, TRUE);
}

// ────── ⋆⋅☆⋅⋆ ────────
// Panel rect helpers
// ────── ⋆⋅☆⋅⋆ ────────

RECT ThreePhaseApp::GetWaveformPanelRect() const {
  RECT rc;
  GetClientRect(hwnd_self, &rc);
  int iSplit = (rc.right * 3) / 5;
  return { 0, CONTROLS_H + 1, iSplit, rc.bottom - READOUT_H };
}

RECT ThreePhaseApp::GetPhasorPanelRect() const {
  RECT rc;
  GetClientRect(hwnd_self, &rc);
  int iSplit = (rc.right * 3) / 5;
  return { iSplit + 1, CONTROLS_H + 1, rc.right, rc.bottom - READOUT_H };
}

// ────── ⋆⋅☆⋅⋆ ────────
// RecalcAndRedraw
// ────── ⋆⋅☆⋅⋆ ────────

void ThreePhaseApp::RecalcAndRedraw() {
  m_inputs.fV_phase_rms = ReadInputFloat(txti_V, 0.0f);
  m_inputs.fFreq_Hz = ReadInputFloat(txti_Freq, 0.0f);
  m_inputs.fR_load = ReadInputFloat(txti_R, 0.0f);
  m_state = Compute(m_inputs);

  UpdateReadouts();

  RECT rcWave = GetWaveformPanelRect();
  RECT rcPhasor = GetPhasorPanelRect();
  InvalidateRect(hwnd_self, &rcWave, TRUE);
  InvalidateRect(hwnd_self, &rcPhasor, TRUE);
}

void ThreePhaseApp::UpdateReadouts() {
  if(!m_state.bValid) {
    lbl_out_Vph->SetText(L"Vph: ---");
    lbl_out_VL->SetText(L"VL: ---");
    lbl_out_Iph->SetText(L"Iph: ---");
    lbl_out_IL->SetText(L"IL: ---");
    lbl_out_P->SetText(L"P: ---");
    lbl_out_neutral->SetText(L"IN: ---");
    lbl_out_config->SetText(L"");
    lbl_status->SetText(L"⚠ Invalid");
    return;
  }

  lbl_status->SetText(L"");

  wchar_t arrBuf[64];

  swprintf(arrBuf, 64, L"Vph: %.1f V", m_state.fV_phase_rms);  
  lbl_out_Vph->SetText(arrBuf);
  swprintf(arrBuf, 64, L"VL: %.1f V", m_state.fV_line_rms);   
  lbl_out_VL->SetText(arrBuf);
  swprintf(arrBuf, 64, L"Iph: %.3f A", m_state.fI_phase_rms);  
  lbl_out_Iph->SetText(arrBuf);
  swprintf(arrBuf, 64, L"IL: %.3f A", m_state.fI_line_rms);   
  lbl_out_IL->SetText(arrBuf);
  swprintf(arrBuf, 64, L"P: %.1f W", m_state.fP_total);       
  lbl_out_P->SetText(arrBuf);

  // Neutral current is only meaningful in star (still 0 for this sim). Hidden in delta
  if(m_state.eConfig == Config::Star) {
    swprintf(arrBuf, 64, L"IN: %.1f A", m_state.fI_neutral);
  } else {
    swprintf(arrBuf, 64, L"IN: N/A");
  }

  lbl_out_neutral->SetText(arrBuf);

  const wchar_t* szConfig = (m_state.eConfig == Config::Star) ? L"Star (Y) neutral present" : L"Delta (Δ) — no neutral";
  lbl_out_config->SetText(szConfig);
}

// ────── ⋆⋅☆⋅⋆ ────────
// ReadInputFloat
// ────── ⋆⋅☆⋅⋆ ────────

float ThreePhaseApp::ReadInputFloat(TextInput* pInput, float fDefault) const {
  wchar_t arrBuf[32];
  pInput->GetText(arrBuf, 32);
  if(arrBuf[0] == L'\0') {
    return fDefault;
  }

  wchar_t* pEnd = nullptr;
  float fVal = wcstof(arrBuf, &pEnd);
  return (pEnd == arrBuf) ? fDefault : fVal;
}