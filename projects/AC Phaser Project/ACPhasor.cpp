/// @file ACPhasor.cpp
#include "ACPhasor.h"
#include <cwchar> // swprintf

ACPhasorApp::ACPhasorApp()
  : lbl_R(nullptr), txti_R(nullptr)
  , lbl_L(nullptr), txti_L(nullptr)
  , lbl_C(nullptr), txti_C(nullptr)
  , lbl_Freq(nullptr), txti_Freq(nullptr)
  , lbl_V(nullptr), txti_V(nullptr)
  , lbl_out_Z(nullptr), lbl_out_Phi(nullptr)
  , lbl_out_PF(nullptr), lbl_out_I(nullptr)
  , lbl_out_P(nullptr), lbl_out_Q(nullptr)
  , lbl_out_S(nullptr), lbl_status(nullptr)
  , m_inputs({}), m_state({})
{}

ACPhasorApp::~ACPhasorApp() {
  delete lbl_R; delete txti_R;
  delete lbl_L; delete txti_L;
  delete lbl_C; delete txti_C;
  delete lbl_Freq; delete txti_Freq;
  delete lbl_V; delete txti_V;
  delete lbl_out_Z; delete lbl_out_Phi;
  delete lbl_out_PF; delete lbl_out_I;
  delete lbl_out_P; delete lbl_out_Q;
  delete lbl_out_S; delete lbl_status;
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnCreate
// ────── ⋆⋅☆⋅⋆ ────────
void ACPhasorApp::OnCreate() {
  HWND hwnd = hwnd_self;

  // Input labels and fields
  lbl_R = new Label(hwnd, L"R (Ω)", 0, 0, 65, 18);
  txti_R = new TextInput(hwnd, ID_INPUT_R, 0, 0, 80, 22);

  lbl_L = new Label(hwnd, L"L (mH):", 0, 0, 65, 18);
  txti_L = new TextInput(hwnd, ID_INPUT_L, 0, 0, 80, 22);

  lbl_C = new Label(hwnd, L"C (µF):", 0, 0, 65, 18);
  txti_C = new TextInput(hwnd, ID_INPUT_C, 0, 0, 80, 22);

  lbl_Freq = new Label(hwnd, L"f (Hz):", 0, 0, 65, 18);
  txti_Freq = new TextInput(hwnd, ID_INPUT_FREQ, 0, 0, 80, 22);

  lbl_V = new Label(hwnd, L"V (Vrms):", 0, 0, 70, 18);
  txti_V = new TextInput(hwnd, ID_INPUT_V, 0, 0, 80, 22);

  // Readout labels
  lbl_out_Z = new Label(hwnd, L"Z: ---", 0, 0, 200, 18);
  lbl_out_Phi = new Label(hwnd, L"φ: ---", 0, 0, 200, 18);
  lbl_out_PF = new Label(hwnd, L"PF: ---", 0, 0, 200, 18);
  lbl_out_I = new Label(hwnd, L"I: ---", 0, 0, 200, 18);
  lbl_out_P = new Label(hwnd, L"P: ---", 0, 0, 200, 18);
  lbl_out_Q = new Label(hwnd, L"Q: ---", 0, 0, 200, 18);
  lbl_out_S = new Label(hwnd, L"S: ---", 0, 0, 200, 18);
  lbl_status = new Label(hwnd, L"", 0, 0, 200, 18);

  // ────── ⋆⋅☆⋅⋆ ────────
  // Seed with values so the app
  // shows a meaningful diagram immediately on launch.
  // R=100Ω, L=100mH, C=100µF, f=60Hz, V=120Vrms
  // ────── ⋆⋅☆⋅⋆ ────────
  txti_R->SetText(L"100");
  txti_L->SetText(L"100");
  txti_C->SetText(L"100");
  txti_Freq->SetText(L"60");
  txti_V->SetText(L"120");

  // Layout and initial calculation
  RECT rc;
  GetClientRect(hwnd, &rc);
  LayoutControls(rc.right, rc.bottom);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnCommand
// ────── ⋆⋅☆⋅⋆ ────────
void ACPhasorApp::OnCommand(int iControlId, int iNotifCode) {
  // EN_CHANGE fiers on every keystroke. Trigger a full recalc on any
  // input field change so the diagram updates live as the user types.
  if(iNotifCode != EN_CHANGE) return;

  switch(iControlId) {
    case ID_INPUT_R:
    case ID_INPUT_L:
    case ID_INPUT_C:
    case ID_INPUT_FREQ:
    case ID_INPUT_V:
      RecalcAndRedraw();
      break;
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnPaint
// ────── ⋆⋅☆⋅⋆ ────────

void ACPhasorApp::OnPaint(HDC hdc) {
  RECT rcClient;
  GetClientRect(hwnd_self, &rcClient);
  int iW = rcClient.right;
  int iH = rcClient.bottom;

  // Window background
  FillRect(hdc, &rcClient, GetSysColorBrush(COLOR_WINDOW));

  // Separator lines
  HPEN hSepPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
  SelectObject(hdc, hSepPen);

  // Below input area
  MoveToEx(hdc, 0, INPUT_AREA_H, NULL);
  LineTo(hdc, iW, INPUT_AREA_H);

  // Abover readout strip
  MoveToEx(hdc, 0, iH - READOUT_AREA_H, NULL);
  LineTo(hdc, iW, iH - READOUT_AREA_H);

  // Between the two GDI panels
  MoveToEx(hdc, iW / 2, INPUT_AREA_H, NULL);
  LineTo(hdc, iW / 2, iH - READOUT_AREA_H);

  DeleteObject(hSepPen);

  // GDI panels
  m_phasorPanel.Draw(hdc, GetPhasorPanelRect(), m_state);
  m_powerPanel.Draw(hdc, GetPowerPanelRect(), m_state);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnMessage
// ────── ⋆⋅☆⋅⋆ ────────

LRESULT ACPhasorApp::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if(uMsg == WM_SIZE) {
    LayoutControls(LOWORD(lParam), HIWORD(lParam));
    InvalidateRect(hwnd_self, NULL, TRUE);
    return 0;
  }
  return Window::OnMessage(uMsg,wParam, lParam);
}

// ────── ⋆⋅☆⋅⋆ ────────
// LayoutControls
// ────── ⋆⋅☆⋅⋆ ────────
void ACPhasorApp::LayoutControls(int iW, int iH) {
  const int iLblW = 65;
  const int iInputW = 80;
  const int iRowH = 22;
  const int iLblH = 18;
  const int iGap = 12; // Gap between label+input groups

  // One label+input pair total width
  const int iGroupW = iLblW + 4 + iInputW;

  // Input row 1: R, L, C
  const int iRow1Y = MARGIN;
  int iTotalW1 = 3 * iGroupW + 2 * iGap;
  int iX = (iW - iTotalW1) / 2;

  MoveWindow(lbl_R->GetHandle(), iX, iRow1Y + 2, iLblW, iLblH, TRUE);
  MoveWindow(txti_R->GetHandle(), iX + iLblW + 4, iRow1Y, iInputW, iRowH, TRUE);
  iX += iGroupW + iGap;

  MoveWindow(lbl_L->GetHandle(), iX, iRow1Y + 2, iLblW, iLblH, TRUE);
  MoveWindow(txti_L->GetHandle(), iX + iLblW + 4, iRow1Y, iInputW, iRowH, TRUE);
  iX += iGroupW + iGap;

  MoveWindow(lbl_C->GetHandle(), iX, iRow1Y + 2, iLblW, iLblH, TRUE);
  MoveWindow(txti_C->GetHandle(), iX + iLblW + 4, iRow1Y, iInputW, iRowH, TRUE);

  // Input row 2: Freq, V
  const int iRow2Y = MARGIN + 34;
  int iTotalW2 = 2 * iGroupW + iGap;
  iX = (iW - iTotalW2) / 2;

  MoveWindow(lbl_Freq->GetHandle(), iX, iRow2Y + 2, iLblW, iLblH, TRUE);
  MoveWindow(txti_Freq->GetHandle(), iX + iLblW + 4, iRow2Y, iInputW, iRowH, TRUE);
  iX += iGroupW + iGap;

  MoveWindow(lbl_V->GetHandle(), iX, iRow2Y + 2, iLblW + 5, iLblH, TRUE);
  MoveWindow(txti_V->GetHandle(), iX + iLblW + 9, iRow2Y, iInputW, iRowH, TRUE);

  // Readout strip
  // Row 1: Z | phi | PF | I
  // Row 2: P | Q | S | status

  const int iColW = iW / 4;
  const int iOutH = 18;
  const int iRdY1 = iH - READOUT_AREA_H + MARGIN;
  const int iRdY2 = iRdY1 + 24;
  const int iOutW = iColW - MARGIN;

  MoveWindow(lbl_out_Z->GetHandle(), 0, iRdY1, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_Phi->GetHandle(), iColW, iRdY1, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_PF->GetHandle(), iColW * 2, iRdY1, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_I->GetHandle(), iColW * 3, iRdY1, iOutW, iOutH, TRUE);

  MoveWindow(lbl_out_P->GetHandle(), 0, iRdY2, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_Q->GetHandle(), iColW, iRdY2, iOutW, iOutH, TRUE);
  MoveWindow(lbl_out_S->GetHandle(), iColW * 2, iRdY2, iOutW, iOutH, TRUE);
  MoveWindow(lbl_status->GetHandle(), iColW * 3, iRdY2, iOutW, iOutH, TRUE);
}

// ────── ⋆⋅☆⋅⋆ ────────
// Panel rect helpers
// ────── ⋆⋅☆⋅⋆ ────────
RECT ACPhasorApp::GetPhasorPanelRect() const {
  RECT rc;
  GetClientRect(hwnd_self, &rc);
  return { 0, INPUT_AREA_H + 1, rc.right / 2, rc.bottom - READOUT_AREA_H };
}

RECT ACPhasorApp::GetPowerPanelRect() const {
  RECT rc;
  GetClientRect(hwnd_self, &rc);
  return { rc.right / 2 + 1, INPUT_AREA_H + 1, rc.right, rc.bottom - READOUT_AREA_H };
}

// ────── ⋆⋅☆⋅⋆ ────────
// RecalcAndRedraw
// ────── ⋆⋅☆⋅⋆ ────────
void ACPhasorApp::RecalcAndRedraw() {
  m_inputs.fR = ReadInputFloat(txti_R, 0.0f);
  m_inputs.fL_mH = ReadInputFloat(txti_L, 0.0f);
  m_inputs.fC_uF = ReadInputFloat(txti_C, 0.0f);
  m_inputs.fFreq_Hz = ReadInputFloat(txti_Freq, 0.0f);
  m_inputs.fV_rms = ReadInputFloat(txti_V, 0.0f);

  m_state = Compute(m_inputs);

  UpdateReadouts();

  // Invalidate onlt the panel region
  // Labels repaint themselves independantly via SetText / MoveWindow
  RECT phasorPanelRect = GetPhasorPanelRect();
  RECT powerPanelRect = GetPowerPanelRect();
  InvalidateRect(hwnd_self, &phasorPanelRect, TRUE);
  InvalidateRect(hwnd_self, &powerPanelRect, TRUE);
}

void ACPhasorApp::UpdateReadouts() {
  if(!m_state.bValid) {
    lbl_out_Z->SetText(L"Z: ---");
    lbl_out_Phi->SetText(L"φ: ---");
    lbl_out_PF->SetText(L"PF: ---");
    lbl_out_I->SetText(L"I: ---");
    lbl_out_P->SetText(L"P: ---");
    lbl_out_Q->SetText(L"Q: ---");
    lbl_out_S->SetText(L"S: ---");
    lbl_status->SetText(L"⚠ Invalid circuit");
    return;
  }

  lbl_status->SetText(L"");
  wchar_t arrBuf[64];
  float fPhi_deg = m_state.fPhi_rad * 180.0f / 3.14159265f;

  swprintf(arrBuf, 64, L"Z: %.2f Ω", m_state.fZ); lbl_out_Z->SetText(arrBuf);
  swprintf(arrBuf, 64, L"φ: %.1f°", fPhi_deg); lbl_out_Phi->SetText(arrBuf);
  swprintf(arrBuf, 64, L"PF: %.3f", m_state.fPowerFactor); lbl_out_PF->SetText(arrBuf);
  swprintf(arrBuf, 64, L"I: %.3f A", m_state.fI_rms); lbl_out_I->SetText(arrBuf);
  swprintf(arrBuf, 64, L"P: %.2f W", m_state.fP); lbl_out_P->SetText(arrBuf);
  swprintf(arrBuf, 64, L"Q: %.2f VAR", m_state.fQ); lbl_out_Q->SetText(arrBuf);
  swprintf(arrBuf, 64, L"S: %.2f VA", m_state.fS); lbl_out_S->SetText(arrBuf);
}

// ────── ⋆⋅☆⋅⋆ ────────
// ReadInputFloat
// ────── ⋆⋅☆⋅⋆ ────────
float ACPhasorApp::ReadInputFloat(TextInput* pInput, float fDefault) const {
  wchar_t arrBuf[32];
  pInput->GetText(arrBuf, 32);
  if(arrBuf[0] == L'\0') return fDefault;

  wchar_t* pEnd = nullptr;
  float fVal = wcstof(arrBuf, &pEnd);

  // pEnd == arrBuf means no valid characters were consumed
  return(pEnd == arrBuf) ? fDefault : fVal;
}