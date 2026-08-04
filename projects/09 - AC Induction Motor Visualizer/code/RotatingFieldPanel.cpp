/// @file RotatingFieldPanel.cpp

#include "RotatingFieldPanel.h"

#ifndef M_PI
#define M_PI  3.14159265f
#endif

#ifndef M_TWOPI
#define M_TWOPI  (2 * M_PI)
#endif

namespace {
  const COLORREF CLR_BG = RGB(12, 14, 26);
  const COLORREF CLR_STATOR = RGB(45, 50, 80); // Stator ring
  const COLORREF CLR_AXIS = RGB(32, 36, 60); // Winding axis lines
  const COLORREF CLR_DIM = RGB(55, 58, 90); // Title / annotations
  const COLORREF CLR_A = RGB(99, 172, 255); // Phase A
  const COLORREF CLR_B = RGB(255, 152,  67); // Phase B
  const COLORREF CLR_C = RGB(72, 199, 142); // Phase C 
  const COLORREF CLR_NET = RGB(240, 240, 255); // Net field
}

void RotatingFieldPanel::Draw(HDC hdc, const RECT& rc, const MotorState& s, float fTheta_rad, bool bForward) {
  int iW = rc.right - rc.left;
  int iH = rc.bottom -rc.top;

  // Off screen buffer
  HDC hdcMem = CreateCompatibleDC(hdc);
  HBITMAP hBmp = CreateCompatibleBitmap(hdc, iW, iH);
  HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmp);

  // Background
  RECT rcLocal = { 0, 0, iW, iH };
  HBRUSH hBg = CreateSolidBrush(CLR_BG);
  FillRect(hdcMem, &rcLocal, hBg);
  DeleteObject(hBg);

  DrawLabel(hdcMem, 8, 6, L"Rotating Magnetic Field", CLR_DIM);

  if(!s.bValid) {
    DrawLabel(hdcMem, iW / 2 - 40, iH / 2 - 8, L"Invalid circuit", CLR_DIM);
    BitBlt(hdc, rc.left, rc.top, iW, iH, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    return;
  }

  // Geometry
  int cx = iW / 2;
  int cy = iH / 2;
  int iR = ((iW < iH) ? iW : iH) / 2 - MARGIN;

  if(iR < 20) {
    BitBlt(hdc, rc.left, rc.top, iW, iH, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    return;
  }

  // Scale
  //   Net field magnitude is always 1.5 Bm
  //   Set scale so 1.5 reaches 90% of the stator radius.
  //   Each winding contribution then reaches 60% radius
  float fScale = (float)iR * 0.6f;

  DrawStator(hdcMem, cx, cy, iR);
  DrawWindingAxes(hdcMem, cx, cy, iR);
  DrawContributions(hdcMem, cx, cy, fScale, fTheta_rad);
  DrawNetField(hdcMem, cx, cy, fScale, fTheta_rad);

  // Annotations
  const wchar_t* szSeq = bForward ? L"Seq: A-B-C (CW)" : L"Seq: A-C-B (CCW)";
  DrawLabel(hdcMem, 8, iH - 34, szSeq, CLR_DIM);

  wchar_t buf[48];
  swprintf(buf, 48, L"\u03C9s = %.1f rad/s", s.fOmega_s);
  DrawLabel(hdcMem, 8, iH - 18, buf, CLR_DIM);

  // Legend
  const int iLx = iW - 80;
  DrawLabel(hdcMem, iLx, 6, L"\u2192 VA", CLR_A);
  DrawLabel(hdcMem, iLx, 20, L"\u2192 VB", CLR_B);
  DrawLabel(hdcMem, iLx, 34, L"\u2192 VC", CLR_C);
  DrawLabel(hdcMem, iLx, 48, L"\u2192 Net", CLR_NET);

  // Blit to screen
  BitBlt(hdc, rc.left, rc.top, iW, iH, hdcMem, 0, 0, SRCCOPY);

  SelectObject(hdcMem, hOldBmp);
  DeleteObject(hBmp);
  DeleteDC(hdcMem);
}

void RotatingFieldPanel::DrawStator(HDC hdc, int cx, int cy, int iR) {
  // Stator ring
  HPEN hPen = CreatePen(PS_SOLID, 2, CLR_STATOR);
  HBRUSH hBr = (HBRUSH)GetStockObject(NULL_BRUSH);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
  HBRUSH hOldBr = (HBRUSH)SelectObject(hdc, hBr);
  Ellipse(hdc, cx - iR, cy - iR, cx + iR, cy + iR);
  SelectObject(hdc, hOldPen);
  SelectObject(hdc, hOldBr);
  DeleteObject(hPen);

  // Center dot
  HPEN hDot = CreatePen(PS_SOLID, 1, CLR_STATOR);
  HBRUSH hSolid = CreateSolidBrush(CLR_STATOR);
  hOldPen = (HPEN)SelectObject(hdc, hDot);
  hOldBr = (HBRUSH)SelectObject(hdc, hSolid);
  Ellipse(hdc, cx - 3, cy - 3, cx + 3, cy + 3);
  SelectObject(hdc, hOldPen);
  SelectObject(hdc, hOldBr);
  DeleteObject(hDot);
  DeleteObject(hSolid);
}

void RotatingFieldPanel::DrawWindingAxes(HDC hdc, int cx, int cy, int iR) {
  const float fAngles[3] = {0.0f, M_TWOPI / 3.0f, 2.0f * M_TWOPI / 3.0f };
  const wchar_t* szLabels[3] = { L"A", L"B", L"C" };
  const COLORREF arrClr[3] = { CLR_A, CLR_B, CLR_C };

  for(int k = 0; k < 3; k++) {
    float fAng = fAngles[k];

    // Axis runs from -R to +R through center
    int ix1 = cx + (int)(iR * cosf(fAng));
    int iy1 = cy - (int)(iR * sinf(fAng));
    int ix2 = cx - (int)(iR * cosf(fAng));
    int iy2 = cy + (int)(iR * sinf(fAng));

    HPEN hPen = CreatePen(PS_DASH, 1, CLR_AXIS);
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, ix1, iy1, NULL);
    LineTo(hdc, ix2, iy2);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);

    // Label just outside the stator ring at the positive end
    const int iOff = 14;
    DrawLabel(hdc, ix1 + (int)(iOff * cosf(fAng)) - 4, iy1 - (int)(iOff * sinf(fAng)) - 8, szLabels[k], arrClr[k]);
  }
}

void RotatingFieldPanel::DrawContributions(HDC hdc, int cx, int cy, float fScale, float fTheta_rad) {
  // Instantaneous phase currents
  float fI[3] = { cosf(fTheta_rad), cosf(fTheta_rad - M_TWOPI / 3.0f), cosf(fTheta_rad - 2.0f * M_TWOPI / 3.0f) };

  // Winding spatial angles
  const float fAngles[3] = { 0.0f, M_TWOPI / 3.0f, 2.0f * M_TWOPI / 3.0f };
  const COLORREF arrClr[3] = { CLR_A, CLR_B, CLR_C };

  for(int k = 0; k < 3; k++) {
    // Contribution vector: fI[k] * unit_vector(fAngles[k]) * scale
    // Negative fI means the arrow reverses along the zxis
    float fEx = fI[k] * cosf(fAngles[k]) * fScale;
    float fEy = fI[k] * sinf(fAngles[k]) * fScale;

    // Skip if too short to be visible
    if(fabsf(fEx) < 2.0f && fabsf(fEy) < 2.0f) {
      continue;
    }

    DrawArrow(hdc, cx, cy, cx + (int)fEx, cy - (int)fEy, arrClr[k], 1); // Y flipped for GDI
  }
}

void RotatingFieldPanel::DrawNetField(HDC hdc, int cx, int cy, float fScale, float fTheta_rad) {
  // Net field is always magnitude 1.5
  // Rotates w/ fTheta_rad
  const float fNetMag = 1.5f;

  float fNx = fNetMag * cosf(fTheta_rad) * fScale;
  float fNy = fNetMag * sinf(fTheta_rad) * fScale;

  DrawArrow(hdc, cx, cy, cx +(int)fNx, cy - (int)fNy, CLR_NET, 2);
}

void RotatingFieldPanel::DrawArrow(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, int iWeight) {
  const int iArrowLen = 10;
  const float fAngle = 0.42f; // ~24°
  
  HPEN hPen = CreatePen(PS_SOLID, iWeight, clr);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  MoveToEx(hdc, x0, y0, NULL);
  LineTo(hdc, x1, y1);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);
  
  float dx = (float)(x1 - x0);
  float dy = (float)(y1 - y0);
  float fLen = sqrtf(dx * dx + dy * dy);
  if (fLen < 1.0f) {
    return; 
  }
  dx /= fLen;
  dy /= fLen;
  
  float ax1 = x1 - iArrowLen * (dx * cosf( fAngle) - dy * sinf( fAngle));
  float ay1 = y1 - iArrowLen * (dy * cosf( fAngle) + dx * sinf( fAngle));
  float ax2 = x1 - iArrowLen * (dx * cosf(-fAngle) - dy * sinf(-fAngle));
  float ay2 = y1 - iArrowLen * (dy * cosf(-fAngle) + dx * sinf(-fAngle));
  
  HPEN hHead = CreatePen(PS_SOLID, iWeight, clr);
  hOld = (HPEN)SelectObject(hdc, hHead);
  MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, (int)ax1, (int)ay1);
  MoveToEx(hdc, x1, y1, NULL); LineTo(hdc, (int)ax2, (int)ay2);
  SelectObject(hdc, hOld);
  DeleteObject(hHead);
}

void RotatingFieldPanel::DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr) {
  COLORREF clrOld = SetTextColor(hdc, clr);
  int iBkOld = SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, x, y, sz, (int)wcslen(sz));
  SetTextColor(hdc, clrOld);
  SetBkMode(hdc, iBkOld);
}