/// @file MotorPabel.cpp

#include "MotorPanel.h"

namespace {
  const COLORREF CLR_BACKGROUND = RGB( 30, 30, 30); // Panel fill
  const COLORREF CLR_BORDER = RGB( 80, 80, 80); // Panel border
  const COLORREF CLR_BAR_EMPTY = RGB( 20, 20, 20); // Unfilled bar
  const COLORREF CLR_BAR_EDGE = RGB( 80, 80, 80); // Bar outline
  const COLORREF CLR_FORWARD = RGB( 50, 200, 100); // Forward fill (green)
  const COLORREF CLR_REVERSE = RGB(200, 80, 50); // Reverse fill (red)
  const COLORREF CLR_CENTER_LINE = RGB(100, 100, 100); // Zero center line
  const COLORREF CLR_LABEL = RGB(200, 200, 200); // Label text
  const COLORREF CLR_READOUT = RGB(180, 180, 180); // Speed readout text
}

MotorPanel::MotorPanel(RECT rcPanel, const wchar_t* szLabel) : rc_panel(rcPanel) {
  wcsncpy(arr_label, szLabel, 31);
  arr_label[31] = L'\0';
}

void MotorPanel::Draw(HDC hdc, int iSpeed) const {
  // Background
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

  const int iLabelH = 26;
  const int iReadoutH = 20;
  const int iMarginX = 14;
  const int iMarginY = 6;

  int iBarLeft = rc_panel.left + iMarginX;
  int iBarRight = rc_panel.right - iMarginX;
  int iBarTop = rc_panel.top + iLabelH + iMarginY;
  int iBarBottom = rc_panel.bottom - iReadoutH - iMarginY;
  int iBarH = iBarBottom - iBarTop;
  int iCenterY = iBarTop + iBarH / 2;

  // Empty bar background
  HBRUSH hBrushEmpty = CreateSolidBrush(CLR_BAR_EMPTY);
  HPEN hPenBarEdge = CreatePen(PS_SOLID, 1, CLR_BAR_EDGE);

  HBRUSH hOldBrushBar = (HBRUSH)SelectObject(hdc, hBrushEmpty);
  HPEN hOldPenBar = (HPEN)SelectObject(hdc, hPenBarEdge);

  Rectangle(hdc, iBarLeft, iBarTop, iBarRight, iBarBottom);

  SelectObject(hdc, hOldBrushBar);
  SelectObject(hdc, hOldPenBar);
  DeleteObject(hBrushEmpty);
  DeleteObject(hPenBarEdge);

  // Filled portion
  //   Map iSpeed [0, 255] to pixel height within one half of the bar
  //   Forward fills upward from center. Reverse fills downward
  if(iSpeed != 0) {
    int iHalfH = iBarH / 2;
    int iAbsSpeed = iSpeed < 0 ? -iSpeed : iSpeed;
    int iFillH = (int)((float)iAbsSpeed / 255.0f * iHalfH);

    COLORREF clrFill = iSpeed > 0 ? CLR_FORWARD : CLR_REVERSE;
    HBRUSH hBrushFill = CreateSolidBrush(clrFill);
    HPEN hPenNull = CreatePen(PS_NULL, 0, 0);

    HBRUSH hOldBrushFill = (HBRUSH)SelectObject(hdc, hBrushFill);
    HPEN hOldPenFill = (HPEN)SelectObject(hdc, hPenNull);

    if(iSpeed > 0) {
      // Forward
      Rectangle( hdc, iBarLeft + 1, iCenterY - iFillH, iBarRight - 1, iCenterY);
    } else {
      // Reverse
      Rectangle(hdc, iBarLeft + 1, iCenterY, iBarRight - 1, iCenterY + iFillH);
    }

    SelectObject(hdc, hOldBrushFill);
    SelectObject(hdc, hOldPenFill);
    DeleteObject(hBrushFill);
    DeleteObject(hPenNull);
  }

  // Center zero line
  HPEN hPenCenter = CreatePen(PS_SOLID, 1, CLR_CENTER_LINE);
  HPEN hOldPenCenter = (HPEN)SelectObject(hdc, hPenCenter);

  MoveToEx(hdc, iBarLeft, iCenterY, NULL);
  LineTo(hdc, iBarRight, iCenterY);

  SelectObject(hdc, hOldPenCenter);
  DeleteObject(hPenCenter);

  // Speed Readou
  SetTextColor(hdc, CLR_READOUT);

  wchar_t arrBuf[16];
  if(iSpeed == 0) {
    wsprintf(arrBuf, L"STOP");
  } else {
    wsprintf(arrBuf, L"%s%d", iSpeed > 0 ? L"+" : L"-", iSpeed < 0 ? -iSpeed : iSpeed);
  }

  RECT rcReadout = {
    rc_panel.left,
    iBarBottom + iMarginY,
    rc_panel.right,
    iBarBottom + iMarginY + 16
  };
  DrawText(hdc, arrBuf, -1, &rcReadout, DT_CENTER | DT_SINGLELINE);
}