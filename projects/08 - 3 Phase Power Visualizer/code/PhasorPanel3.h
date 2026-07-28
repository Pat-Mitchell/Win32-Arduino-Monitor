/// @file PhasorPanel3.h

#pragma once
#include <windows.h>
#include <cmath>
#include "ThreePhaseMath.h"

class PhasorPanel3 {
  public:
    void Draw(HDC hdc, const RECT& rc, const ThreePhaseState& s);

  private:
    /// @brief Draws an arrowed line from (x0,y0) to (x1,y1)
    ///   iWeight controls shaft and arrowhead thickness
    void DrawArrow(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, int iWeight = 2);
    void DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr);

    static const int MARGIN = 28;
};