/// @file TriggerPanel.cpp
/// @brief Implementation of the TriggerPanel GDI visualizer

#include "TriggerPanel.h"

// ────── ⋆⋅☆⋅⋆ ────────
// GDI Color Palette
// ────── ⋆⋅☆⋅⋆ ────────
namespace {
  const COLORREF CLR_BACKGROUND = RGB(30, 30, 30);
  const COLORREF CLR_BORDER = RGB(80, 80, 80);
  const COLORREF CLR_BAR_FILL = RGB(200, 140, 30);
  const COLORREF CLR_BAR_EMPTY = RGB(20, 20, 20);
  const COLORREF CLR_BAR_EDGE = RGB(80, 80, 80);
  const COLORREF CLR_LABEL = RGB(200, 200, 200);
  const COLORREF CLR_READOUT = RGB(220, 180, 100);
}

TriggerPanel::TriggerPanel(RECT rcPanel, const wchar_t* szAxisPrefix) : rc_panel(rcPanel) {
  wcsncpy(arr_axis_prefix, szAxisPrefix, 3);
  arr_axis_prefix[3] = L'\0';
}

void TriggerPanel::Draw(HDC hdc, BYTE bValue) const {
  int iPanelW = rc_panel.right - rc_panel.left;

  // Background and border
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

  wchar_t arrTitle[32];
  wsprintf(arrTitle, L"%sTRIGGER", arr_axis_prefix[0] == L'L' ? L"LEFT " : L"RIGHT ");
  
  RECT rcLabel = {
    rc_panel.left,
    rc_panel.top + 6,
    rc_panel.right,
    rc_panel.top + 22
  };
  DrawText(hdc, arrTitle, -1, &rcLabel, DT_CENTER | DT_SINGLELINE);

  // Bar geometry
  const int iLabelH = 26;
  const int iReadoutH = 20;
  const int iMarginX = 10;
  const int iMarginY = 6;

  int iBarLeft = rc_panel.left + iMarginX;
  int iBarRight = rc_panel.right - iMarginX;
  int iBarTop = rc_panel.top + iLabelH + iMarginY;
  int iBarBottom = rc_panel.bottom - iReadoutH - iMarginY;
  int iBarW = iBarRight - iBarLeft;

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
  // Map bValue [0, 255] to pixel width within the bar
  
  if(bValue > 0) {
    int iFillW = (int)((float)bValue / 255.0f * iBarW);

    HBRUSH hBrushFill = CreateSolidBrush(CLR_BAR_FILL);
    HPEN hPenNull = CreatePen(PS_NULL, 0, 0);

    HBRUSH hOldBrushFill = (HBRUSH)SelectObject(hdc, hBrushFill);
    HPEN hOldPenFill = (HPEN)SelectObject(hdc, hPenNull);

    // +1 on iBarLeft so the fill sits inside the bar outline
    Rectangle(hdc, iBarLeft + 1, iBarTop + 1, iBarLeft + iFillW, iBarBottom - 1);

    SelectObject(hdc, hOldBrushFill);
    SelectObject(hdc, hOldPenFill);
    DeleteObject(hBrushFill);
    DeleteObject(hPenNull);
  }

  // Numeric readout
  SetTextColor(hdc, CLR_READOUT);

  wchar_t arrBuf[16];
  wsprintf(arrBuf, L"%sT: %3d", arr_axis_prefix, (int)bValue);

  RECT rcReadout = {
    rc_panel.left,
    iBarBottom + iMarginY,
    rc_panel.right,
    iBarBottom + iMarginY + 16
  };
  DrawText(hdc, arrBuf, -1, &rcReadout, DT_CENTER | DT_SINGLELINE);
}