/// @file RotatingFieldPanel.h
/// @brief GDI panel that animates the rotating magnetic field of a
///   three-phase induction motor stator.

#pragma once
#include <windows.h>
#include <cmath>
#include <cwchar>
#include "MotorMath.h"

class RotatingFieldPanel {
  public:
    void Draw(HDC hdc, const RECT& rc, const MotorState& s, float fTheta_rad, bool bForward = true);

  private:
    void DrawStator(HDC hdcMem, int cx, int cy, int iR);
    void DrawWindingAxes(HDC hdcMem, int cx, int cy, int iR);
    void DrawContributions(HDC hdcMem, int cx, int cy, float fScale, float fTheta_rad);
    void DrawNetField(HDC hdcMem, int cx, int cy, float fScale, float fTheta_rad);
    void DrawArrow(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, int iWeight);
    void DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr);

    static const int MARGIN = 28;
};