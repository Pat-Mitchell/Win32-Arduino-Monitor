/// @file RotatingFieldPanel.h
/// @brief Stubbed

#pragma once
#include <windows.h>
#include "MotorMath.h"

class RotatingFieldPanel {
  public:
    void Draw(HDC hdc, const RECT& rc, const MotorState& s, float fTheta_rad) {
      HBRUSH hBg = CreateSolidBrush(RGB(12, 14, 26));
      FillRect(hdc, &rc, hBg);
      DeleteObject(hBg);
      SetTextColor(hdc, RGB(80, 80, 110));
      SetBkMode(hdc, TRANSPARENT);
      RECT rText = rc;
      DrawText(hdc, L"Rotating field", -1, &rText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
};