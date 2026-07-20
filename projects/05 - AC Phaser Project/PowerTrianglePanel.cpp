/// @file PowerTrianglePanel.cpp
#include "PowerTrianglePanel.h"
#include <cwchar>
#include <cmath>
#define min(a,b) (((a) < (b)) ? (a) : (b))

namespace {
  const COLORREF CLR_BG = RGB(12, 14, 26); // Panel background
  const COLORREF CLR_P = RGB(72, 199, 142); // Real power — green
  const COLORREF CLR_Q = RGB(255, 152, 67); // Reactive power — amber
  const COLORREF CLR_S = RGB(240, 240, 255); // Apparent power — near white
  const COLORREF CLR_DIM = RGB(55, 58, 90); // Panel title / fallback text
  const COLORREF CLR_RA = RGB(60, 65, 100); // Right-angle marker
}

void PowerTrianglePanel::Draw(HDC hdc, const RECT& rc, const PhasorState& s) {
  // Background
  HBRUSH hBg = CreateSolidBrush(CLR_BG);
  FillRect(hdc, &rc, hBg);
  DeleteObject(hBg);

  int iW = rc.right - rc.left;
  int iH = rc.bottom - rc.top;

  DrawLabel(hdc, rc.left + 8, rc.top + 6, L"Power Triangle", CLR_DIM);

  if(!s.bValid || s.fS < 1e-6f) {
    DrawLabel(hdc, rc.left + iW / 2 - 50, rc.top + iH / 2 - 8, L"No power to display", CLR_DIM);
    return;
  }

  // Scale
  // Origin at center-left. P extends right; Q extends up or down from end of P.
  // S (hypotenuse) goes from origin directly to the far orner.
  int ox = rc.left + MARGIN;
  int oy = rc.top + iH / 2;

  int iAvailW = (rc.right - MARGIN) - ox;
  int iAvailH = iH / 2 - MARGIN;
  float fAbsQ = fabsf(s.fQ);

  // Scale horizontally to S (hypotenuse)
  // Scale vertically to |Q| if reactive power is present
  float fScaleH = (float)iAvailW / s.fP;
  float fScaleV = (fAbsQ > 1e-6f) ? (float)iAvailH / fAbsQ : fScaleH;
  float fScale = min(fScaleH, fScaleV);

  // Triangle vertices
  // A - origin
  // B - right end of P
  // C - tip of Q from B
  int iAx = ox;
  int iAy = oy;
  int iBx = ox + (int)(s.fP * fScale);
  int iBy = oy;

  // s.fQ is signed: positive (inductive)
  //                 negative (capacitive)
  int iCx = iBx;
  int iCy = oy - (int)(s.fQ * fScale);

  // Right-angle marker at B
  // Only draw if P is long enough for the marker to fit cleanly
  const int iRaSize = 8;
  int iPxLen = iBx - iAx;
  if(iPxLen > iRaSize + 2) {
    int iRaDir = (s.fQ > 0.0f) ? -1 : 1; // -1 = up in screen coords
    HPEN hRa = CreatePen(PS_SOLID, 1, CLR_RA);
    HPEN hOld = (HPEN)SelectObject(hdc, hRa);
    MoveToEx(hdc, iBx - iRaSize, iBy, NULL);
    LineTo(hdc, iBx - iRaSize, iBy + iRaDir * iRaSize);
    LineTo(hdc, iBx, iBy + iRaDir * iRaSize);
    SelectObject(hdc, hOld);
    DeleteObject(hRa);
  }

  // Sides
  DrawSide(hdc, iAx, iAy, iBx, iBy, CLR_P, 3); // P; base (horizontal)

  if(fAbsQ * fScale > 2.0f) {
    DrawSide(hdc, iBx, iBy, iCx, iCy, CLR_Q, 3); // Q; vertical
  }

  DrawSide(hdc, iAx, iAy, iCx, iCy, CLR_S, 2); // S; hypotenuse

  // Side labels
  // P: centred below the base
  DrawSideLabel(hdc, (iAx + iBx) / 2 - 30, iAy + 14, L"P", s.fP, L"W", CLR_P);

  // Q: to the right of the vertical, only if visible
  if (fAbsQ * fScale > 2.0f)
    DrawSideLabel(hdc, iCx + 8, (iBy + iCy) / 2 - 8, L"Q", s.fQ, L"VAR", CLR_Q);

  // S: offset perpendicular (left of the hypotenuse direction)
  float fSdx = (float)(iCx - iAx);
  float fSdy = (float)(iCy - iAy);
  float fSln = sqrtf(fSdx * fSdx + fSdy * fSdy);
  if(fSln > 1.0f) {
    // Left perpendicular of S: (-dy, dx) normalised
    float fNx = -fSdy / fSln;
    float fNy =  fSdx / fSln;
    int iSlx = (iAx + iCx) / 2 + (int)(fNx * 20) - 16;
    int iSly = (iAy + iCy) / 2 + (int)(fNy * 20) - 8;
    DrawSideLabel(hdc, iSlx, iSly, L"S", s.fS, L"VA", CLR_S);
  }
}

void PowerTrianglePanel::DrawSide(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, int iWeight) {
  HPEN hPen = CreatePen(PS_SOLID, iWeight, clr);
  HPEN hOld = (HPEN)SelectObject(hdc, hPen);
  MoveToEx(hdc, x0, y0, NULL);
  LineTo(hdc, x1, y1);
  SelectObject(hdc, hOld);
  DeleteObject(hPen);
}

void PowerTrianglePanel::DrawSideLabel(HDC hdc, int x, int y, const wchar_t* szName, float fVal, const wchar_t* szUnit, COLORREF clr) {
  wchar_t arrBuf[48];
  swprintf(arrBuf, 48, L"%s: %.1f %s", szName, fabsf(fVal), szUnit);
  DrawLabel(hdc, x, y, arrBuf, clr);
}

void PowerTrianglePanel::DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr) {
  COLORREF clrOld = SetTextColor(hdc, clr);
  int iBkOld = SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, x, y, sz, (int)wcslen(sz));
  SetTextColor(hdc, clrOld);
  SetBkMode(hdc, iBkOld);
}