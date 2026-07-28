/// @file WaveformPanel.cpp

#include "WaveformPanel.h"
#include <cwchar>

#ifndef M_PI
#define M_PI  3.14159265
#endif

namespace {
  const COLORREF CLR_BG = RGB(12,  14,  26);
  const COLORREF CLR_ZERO = RGB(40,  44,  70);   // Horizontal zero line
  const COLORREF CLR_GRID = RGB(28,  31,  52);   // Vertical grid lines
  const COLORREF CLR_DIM = RGB(55,  58,  90);   // Title/fallback text
  const COLORREF CLR_A = RGB(99,  172, 255);  // Phase A 
  const COLORREF CLR_B = RGB(255, 152,  67);  // Phase B 
  const COLORREF CLR_C = RGB(72,  199, 142);  // Phase C 
}

void WaveformPanel::Draw(HDC hdc, const RECT& rc, const ThreePhaseState& s) {
  // Background
  HBRUSH hBg = CreateSolidBrush(CLR_BG);
  FillRect(hdc, &rc, hBg);
  DeleteObject(hBg);

  int iW = rc.right - rc.left;
  int iH = rc.bottom - rc.top;
  int ox = rc.left + MARGIN;
  int oy = rc.top + iH / 2;
  int iAvailW = (rc.right - MARGIN) - ox;
  int iAvailH = iH / 2 - MARGIN;

  DrawLabel(hdc, rc.left + 8, rc.top + 6, L"Waveform", CLR_DIM);

  if(!s.bValid) {
    DrawLabel(hdc, rc.left + iW / 2 - 40, oy - 8, L"Invalid circuit", CLR_DIM);
    return;
  }

  DrawAxes(hdc, rc, ox, oy, iAvailW);

  // Scale. Peak voltage must fit within the half-height
  float fScale = (s.fV_phase_peak > 1e-6f) ? (float)iAvailH / s.fV_phase_peak : 1.0f;

  // Draw order: C, B, A. A is last, so it appears on top
  DrawWave(hdc, ox, oy, iAvailW, s.fV_phase_peak, fScale, s.fOffset_C_rad, CLR_C);
  DrawWave(hdc, ox, oy, iAvailW, s.fV_phase_peak, fScale, s.fOffset_B_rad, CLR_B);
  DrawWave(hdc, ox, oy, iAvailW, s.fV_phase_peak, fScale, s.fOffset_A_rad, CLR_A);

  // Labels at each wave's peak x-position. Naturally eparated for all valid
  // offset combinations. Peak position for ABC: A at 0deg, B at 120deg c at 240deg
  // For ACB they swap
  DrawWaveLabel(hdc, ox, oy, iAvailW, s.fV_phase_peak, fScale, s.fOffset_A_rad, L"A", CLR_A);
  DrawWaveLabel(hdc, ox, oy, iAvailW, s.fV_phase_peak, fScale, s.fOffset_B_rad, L"B", CLR_B);
  DrawWaveLabel(hdc, ox, oy, iAvailW, s.fV_phase_peak, fScale, s.fOffset_C_rad, L"C", CLR_C);
}

void WaveformPanel::DrawAxes(HDC hdc, const RECT& rc, int ox, int oy, int iAvailW) {
  const wchar_t* arrDegLabels[] = { L"0°", L"120°", L"240°", L"360°" };

  // Vertical grid lines a 0, 120, 240, 360
  HPEN hGrid = CreatePen(PS_SOLID, 1, CLR_GRID);
  HPEN hOld = (HPEN)SelectObject(hdc, hGrid);
  for(int i = 0; i <= 3; i++) {
    int iGx = ox + (iAvailW * i) / 3;
    MoveToEx(hdc, iGx, rc.top + 4, NULL);
    LineTo(hdc, iGx, rc.bottom - 4);
  }
  SelectObject(hdc, hOld);
  DeleteObject(hGrid);

  // Horizontal zero lines
  HPEN hZero = CreatePen(PS_SOLID, 1, CLR_ZERO);
  hOld = (HPEN)SelectObject(hdc, hZero);
  MoveToEx(hdc, ox, oy, NULL);
  LineTo(hdc, ox + iAvailW, oy);
  SelectObject(hdc, hOld);
  DeleteObject(hZero);

  // Degeree labels just below the zero line
  for(int i = 0; i <= 3; i++) {
    int iGx = ox + (iAvailW * i) / 3;
    DrawLabel(hdc, iGx - 8, oy + 4, arrDegLabels[i], CLR_DIM);
  }
}

void WaveformPanel::DrawWave(HDC hdc, int ox, int oy, int iAvailW, float fPeak, float fScale, float fOffset_rad, COLORREF clr) {
  const float fTwoPi = 2.0f * M_PI;
  POINT pts[STEPS + 1];

  for(int i = 0; i <= STEPS; i++) {
    float fTheta = fTwoPi * i / (float)STEPS;
    float fV = fPeak * cosf(fTheta + fOffset_rad);
    pts[i].x = ox + (iAvailW * i) / STEPS;
    pts[i].y = oy - (int)(fV * fScale);
  }

  HPEN hPen = CreatePen(PS_SOLID, 2, clr);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  Polyline(hdc, pts, STEPS + 1);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);
}

void WaveformPanel::DrawWaveLabel(HDC hdc, int ox, int oy, int iAvailW, float fPeak, float fScale, float fOffset_rad, const wchar_t* sz, COLORREF clr) {
  const float fTwoPi = 2.0f * M_PI;
  float fTheta = -fOffset_rad;

  while(fTheta < 0.0f) {
    fTheta += fTwoPi;
  }

  while(fTheta >= fTwoPi) {
    fTheta -= fTwoPi;
  }

  int iLx = ox + (int)((fTheta / fTwoPi) * iAvailW);
  int iLy = oy - (int)(fPeak * fScale) - 16;

  DrawLabel(hdc, iLx - 4, iLy, sz, clr);
}

void WaveformPanel::DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr) {
  COLORREF clrOld = SetTextColor(hdc, clr);
  int iBkOld = SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, x, y, sz, (int)wcslen(sz));
  SetTextColor(hdc, clrOld);
  SetBkMode(hdc, iBkOld);
}