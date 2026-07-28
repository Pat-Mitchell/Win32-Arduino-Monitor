/// @file PhasorPanel3.cpp

#include "PhasorPanel3.h"
#include <cwchar>

namespace {
    const COLORREF CLR_BG   = RGB(12,  14,  26);
    const COLORREF CLR_AXIS = RGB(35,  38,  65);
    const COLORREF CLR_DIM  = RGB(55,  58,  90);
    const COLORREF CLR_A    = RGB(99,  172, 255);  // Phase A
    const COLORREF CLR_B    = RGB(255, 152,  67);  // Phase B
    const COLORREF CLR_C    = RGB(72,  199, 142);  // Phase C
    const COLORREF CLR_LINE = RGB(190, 190, 215);  // Line voltages
}

void PhasorPanel3::Draw(HDC hdc, const RECT& rc, const ThreePhaseState& s) {
  // Background
  HBRUSH hBg = CreateSolidBrush(CLR_BG);
  FillRect(hdc, &rc, hBg);
  DeleteObject(hBg);

  int iW = rc.right - rc.left;
  int iH = rc.bottom - rc.top;
  int ox = rc.left + iW / 2; // Origin at panel center
  int oy = rc.top + iH / 2;

  DrawLabel(hdc, rc.left + 8, rc.top + 6, L"Phasor Diagram", CLR_DIM);

  // Crosshair axes through origin
  HPEN hAx = CreatePen(PS_SOLID, 1, CLR_AXIS);
  HPEN hOld = (HPEN)SelectObject(hdc, hAx);
  MoveToEx(hdc, rc.left + 4, oy, NULL);
  LineTo(hdc, rc.right - 4, oy);
  MoveToEx(hdc, ox, rc.top + 4, NULL);
  LineTo(hdc, ox, rc.bottom - 4);
  SelectObject(hdc, hOld);
  DeleteObject(hAx);

  if(!s.bValid) {
    DrawLabel(hdc, ox - 40, oy - 8, L"Invalid circuit", CLR_DIM);
    return;
  }

  // Scale
  // Largest circle that fits inside the panel minus margin
  // Phase phasor tips are always on this circle. Lone voltage tips
  // are between phase tips so they never exceed this radius.
  int iRadius = ((iW < iH) ? iW : iH) / 2 - MARGIN;
  if(iRadius < 10) return;

  float fScale = (s.fV_phase_rms > 1e-6f) ? (float)iRadius / s.fV_phase_rms : 1.0f;

  // Phase phasor tip screen coordinates
  // screen_x = ox + V * cos(offset) * scale
  // screen_y = ox - V * sin(offset) * scale (Y axis flipped)
  int iAx = ox + (int)(s.fV_phase_rms * cosf(s.fOffset_A_rad) * fScale);
  int iAy = oy - (int)(s.fV_phase_rms * sinf(s.fOffset_A_rad) * fScale);

  int iBx = ox + (int)(s.fV_phase_rms * cosf(s.fOffset_B_rad) * fScale);
  int iBy = oy - (int)(s.fV_phase_rms * sinf(s.fOffset_B_rad) * fScale);

  int iCx = ox + (int)(s.fV_phase_rms * cosf(s.fOffset_C_rad) * fScale);
  int iCy = oy - (int)(s.fV_phase_rms * sinf(s.fOffset_C_rad) * fScale);

  // Line voltage phasors
  //   drawn first behind phasors
  // V_AB: tip_A -> tip_B (geometrically A_AB = V_A - V_B)
  // V_BC: tip_B -> tip_C
  // V_CA: tip_C -> tip_A
  // Weight 1 so they read as secondary to the phase phasor
  DrawArrow(hdc, iAx, iAy, iBx, iBy, CLR_LINE, 1);
  DrawArrow(hdc, iBx, iBy, iCx, iCy, CLR_LINE, 1);
  DrawArrow(hdc, iCx, iCy, iAx, iAy, CLR_LINE, 1);

  // Line voltage labels at midpoint of each arrow. Offset slightly above
  DrawLabel(hdc, (iAx + iBx) / 2 + 4, (iAy + iBy) / 2 - 14, L"VAB", CLR_LINE);
  DrawLabel(hdc, (iBx + iCx) / 2 + 4, (iBy + iCy) / 2 - 14, L"VBC", CLR_LINE);
  DrawLabel(hdc, (iCx + iAx) / 2 + 4, (iCy + iAy) / 2 - 14, L"VCA", CLR_LINE);

  // Phase phasors from origin
  DrawArrow(hdc, ox, oy, iAx, iAy, CLR_A);
  DrawArrow(hdc, ox, oy, iBx, iBy, CLR_B);
  DrawArrow(hdc, ox, oy, iCx, iCy, CLR_C);

  // Phase phasor labels. Offset outward along the phasor direction
  const int iLblOff = 8;
  DrawLabel(hdc, iAx + (int)(iLblOff * cosf(s.fOffset_A_rad)), iAy - (int)(iLblOff * sinf(s.fOffset_A_rad)) - 8, L"VA", CLR_A);
  DrawLabel(hdc, iBx + (int)(iLblOff * cosf(s.fOffset_B_rad)), iBy - (int)(iLblOff * sinf(s.fOffset_B_rad)) - 8, L"VB", CLR_B);
  DrawLabel(hdc, iCx + (int)(iLblOff * cosf(s.fOffset_C_rad)), iCy - (int)(iLblOff * sinf(s.fOffset_C_rad)) - 8, L"VC", CLR_C);

  // Phase sequence label — bottom left corner
  const wchar_t* szSeq = (s.eSeq == PhaseSeq::ABC) ? L"Seq: A-B-C" : L"Seq: A-C-B";
  DrawLabel(hdc, rc.left + 8, rc.bottom - 20, szSeq, CLR_DIM);
}

void PhasorPanel3::DrawArrow(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, int iWeight) {
  const int iArrowLen = 10;
  const float fAngle = 0.42f; // ~24deg

  // shaft
  HPEN hPen = CreatePen(PS_SOLID, iWeight, clr);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  MoveToEx(hdc, x0, y0, NULL);
  LineTo(hdc, x1, y1);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);

  // Guard. Skip arrowhead if the line is too short to be visible
  float dx = (float)(x1 - x0);
  float dy = (float)(y1 - y0);
  float fLen = sqrtf(dx * dx + dy * dy);
  if(fLen < 1.0f) return;
  dx /= fLen;
  dy /= fLen;

  // Arrowhead. Two lines rotated fAngle from the reverse shaft direction
  float ax1 = x1 - iArrowLen * (dx * cosf(fAngle) - dy * sinf(fAngle));
  float ay1 = y1 - iArrowLen * (dy * cosf(fAngle) + dx * sinf(fAngle));
  float ax2 = x1 - iArrowLen * (dx * cosf(-fAngle) - dy * sinf(-fAngle));
  float ay2 = y1 - iArrowLen * (dy * cosf(-fAngle) + dx * sinf(-fAngle));

  HPEN hHead = CreatePen(PS_SOLID, iWeight, clr);
  hOld =(HPEN)SelectObject(hdc, hHead);
  MoveToEx(hdc, x1, y1, NULL);
  LineTo(hdc, (int)ax1, (int)ay1);
  MoveToEx(hdc, x1, y1, NULL);
  LineTo(hdc, (int)ax2, (int)ay2);
  SelectObject(hdc, hOld);
  DeleteObject(hHead);
}

void PhasorPanel3::DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr) {
  COLORREF clrOld = SetTextColor(hdc, clr);
  int iBkOld = SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, x, y, sz, (int)wcslen(sz));
  SetTextColor(hdc, clrOld);
  SetBkMode(hdc, iBkOld);
}