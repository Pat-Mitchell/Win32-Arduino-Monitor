/// @file WaveformPanel.h

#pragma once
#include <windows.h>
#include "ThreePhaseMath.h"

class WaveformPanel {
  public:
    void Draw(HDC hdc, const RECT& rc, const ThreePhaseState& s) {
      HBRUSH hBg = CreateSolidBrush(RGB(12, 14, 26));
      FillRect(hdc, &rc, hBg);
      DeleteObject(hBg);
      SetTextColor(hdc, RGB(80, 80, 110));
      SetBkMode(hdc, TRANSPARENT);
      RECT rText = rc;
      DrawText(hdc, L"Waveform Panel", -1, &rText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
};