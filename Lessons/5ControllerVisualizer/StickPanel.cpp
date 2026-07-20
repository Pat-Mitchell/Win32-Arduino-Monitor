/// @file StickPanel.cpp
/// @brief Implementation of the StickPanel GDI visualizer

#include "StickPanel.h"
#include <math.h> // sqrtf

// ────── ⋆⋅☆⋅⋆ ────────
// GDI color palette
// ────── ⋆⋅☆⋅⋆ ────────
namespace {
  const COLORREF CLR_BACKGROUND = RGB(30, 30, 30);
  const COLORREF CLR_BORDER = RGB(80, 80, 80);
  const COLORREF CLR_CIRCLE_FILL = RGB(20, 20, 20);
  const COLORREF CLR_CIRCLE_EDGE = RGB(100, 100, 100);
  const COLORREF CLR_DEADZONE = RGB(80, 60, 20);
  const COLORREF CLR_DOT = RGB(50, 200, 100);
  const COLORREF CLR_CROSSHAIR = RGB(55, 55, 55);
  const COLORREF CLR_LABEL = RGB(200, 200, 200);
  const COLORREF CLR_READOUT = RGB(160, 210, 160);
}

StickPanel::StickPanel(RECT rcPanel, const wchar_t* szLabel, const wchar_t* szAxisPrefix) : rc_panel(rcPanel) {
  wcsncpy(arr_label, szLabel, 31);
  arr_label[31] = L'\0';

  wcsncpy(arr_axis_prefix, szAxisPrefix, 3);
  arr_axis_prefix[3] = L'\0';
}

void StickPanel::Draw(HDC hdc, SHORT sFilteredX, SHORT sFilteredY, SHORT sRawX, SHORT sRawY, int iDeadzone) const {
  int iPanelW = rc_panel.right - rc_panel.left;
  int iPanelH = rc_panel.bottom - rc_panel.top;

  // Panel background and border
  HBRUSH hBrushBg = CreateSolidBrush(CLR_BACKGROUND);
  HPEN hPenBorder = CreatePen(PS_SOLID, 1, CLR_BORDER);

  HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushBg);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);

  Rectangle(hdc, rc_panel.left, rc_panel.top, rc_panel.right, rc_panel.bottom);

  SelectObject(hdc, hOldBrush);
  SelectObject(hdc, hOldPen);
  DeleteObject(hBrushBg);
  DeleteObject(hPenBorder);

  // Label
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, CLR_LABEL);

  RECT rcLabel = {
    rc_panel.left,
    rc_panel.top + 6,
    rc_panel.right,
    rc_panel.top + 22
  };
  
  DrawText(hdc, arr_label, -1, &rcLabel, DT_CENTER | DT_SINGLELINE);

  // Circle
  const int iReadoutH = 36; // Pixels below circle for LX/LY text
  const int iLabelH = 26; // Pixels consumed by label above circle
  const int iMargin = 6; // Padding around the circle

  int iAvailH = iPanelH - iLabelH - iReadoutH - (iMargin * 2);
  int iAvailW = iPanelW - (iMargin * 2);
  int iRadius = (iAvailH < iAvailW ? iAvailH : iAvailW) / 2;

  POINT ptCenter = {
    rc_panel.left + iPanelW / 2,
    rc_panel.top + iLabelH + iMargin + iRadius
  };

  // Circle fill
  HBRUSH hBrushCircle = CreateSolidBrush(CLR_CIRCLE_FILL);
  HPEN hPenCircleEdge = CreatePen(PS_SOLID, 1, CLR_CIRCLE_EDGE);

  HBRUSH hOldBrushCircle = (HBRUSH)SelectObject(hdc, hBrushCircle);
  HPEN hOldPenCircle = (HPEN)SelectObject(hdc, hPenCircleEdge);

  Ellipse(hdc, ptCenter.x - iRadius, ptCenter.y - iRadius, ptCenter.x + iRadius, ptCenter.y + iRadius);

  SelectObject(hdc, hOldBrushCircle);
  SelectObject(hdc, hOldPenCircle);
  DeleteObject(hBrushCircle);
  DeleteObject(hPenCircleEdge);

  // Crosshair
  HPEN hPenCross = CreatePen(PS_SOLID, 1, CLR_CROSSHAIR);
  HPEN hOldPenCross = (HPEN)SelectObject(hdc, hPenCross);

  MoveToEx(hdc, ptCenter.x - iRadius, ptCenter.y, NULL);
  LineTo(hdc, ptCenter.x + iRadius, ptCenter.y);

  MoveToEx(hdc, ptCenter.x, ptCenter.y -iRadius, NULL);
  LineTo(hdc, ptCenter.x, ptCenter.y + iRadius);

  SelectObject(hdc, hOldPenCross);
  DeleteObject(hPenCross);

  // Deadzone ring
  // Map iDeadzone (0-32767) to a pixel radius within the circle
  int iDzRadius = (int)((float)iDeadzone / 32767.0f * iRadius);

  HPEN hPenDz = CreatePen(PS_SOLID, 1, CLR_DEADZONE);
  HBRUSH hOldBrushDz = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
  HPEN hOldPenDz = (HPEN)SelectObject(hdc, hPenDz);

  Ellipse(hdc, ptCenter.x - iDzRadius, ptCenter.y - iDzRadius, ptCenter.x + iDzRadius, ptCenter.y + iDzRadius);

  SelectObject(hdc, hOldBrushDz);
  SelectObject(hdc, hOldPenDz);
  DeleteObject(hPenDz);

  // Stick position dot
  // Clip to circle radius so the dot never escapes the field
  POINT ptDot;
  MapStickToCircle(sFilteredX, sFilteredY, ptCenter, iRadius, ptDot);

  const int iDotR = 5;
  HBRUSH hBrushDot = CreateSolidBrush(CLR_DOT);
  HPEN hPenDot = CreatePen(PS_SOLID, 1, CLR_DOT);
  HBRUSH hOldBrushDot = (HBRUSH) SelectObject(hdc, hBrushDot);
  HPEN hOldPenDot = (HPEN)SelectObject(hdc, hPenDot);

  Ellipse(hdc, ptDot.x - iDotR, ptDot.y - iDotR, ptDot.x + iDotR, ptDot.y + iDotR);

  SelectObject(hdc, hOldBrushDot);
  SelectObject(hdc, hOldPenDot);
  DeleteObject(hBrushDot);
  DeleteObject(hPenDot);

  // Raw axis readouts
  SetTextColor(hdc, CLR_READOUT);

  wchar_t arrBuf[32];
  int iReadoutTop = ptCenter.y + iRadius + iMargin + 2;

  wsprintf(arrBuf, L"%sX: %6d", arr_axis_prefix, (int)sRawX);
  RECT rcRX = {
    rc_panel.left,
    iReadoutTop,
    rc_panel.right,
    iReadoutTop + 16
  };
  DrawText(hdc, arrBuf, -1, &rcRX, DT_CENTER | DT_SINGLELINE);

  wsprintf(arrBuf, L"%sY: %6d", arr_axis_prefix, (int)sRawY);
  RECT rcRY = {
    rc_panel.left,
    iReadoutTop + 18,
    rc_panel.right,
    iReadoutTop + 34
  };
  DrawText(hdc, arrBuf, -1, &rcRY, DT_CENTER | DT_SINGLELINE);
}

// Private

void StickPanel::MapStickToCircle(SHORT sX, SHORT sY, POINT ptCenter, int iRadius, POINT& ptOut) {
  // Normalise to [-1.0, +1.0]
  float fX = (float)sX / 32767.0f;
  float fY = -(float)sY / 32767.0f; // Invert Y. XInput up = +, GDI up = -

  // Clamp magnitude to 1.0 so the dot never leaves the circle
  float fMag = sqrtf(fX * fX + fY * fY);
  if(fMag > 1.0f) {
    fX /= fMag;
    fY /= fMag;
  }

  ptOut.x = ptCenter.x + (int)(fX * (iRadius - 6));
  ptOut.y = ptCenter.y + (int)(fY * (iRadius - 6));
}