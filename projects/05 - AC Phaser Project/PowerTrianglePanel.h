/// @file PowerTrianglePanel.h
/// @brief Draws the power triangle (P/Q/S) for a series RLC circuit.
///   Right angle sits at the end of P. Q extends up (inductive)
///   or down (capacitive) based on the sign of PhasorState::fQ.

#pragma once
#include <windows.h>
#include "phasorMath.h"

class PowerTrianglePanel {
  public:
    void Draw(HDC hdc, const RECT& rc, const PhasorState& s);

  private:
    /// @brief Draws a solid line from (x0, y0) to (x1, y1) in the given color
    void DrawSide(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, int iWeight);

    /// @brief Formats "NAME: VALUE UNIT" and renders it at (x, y).
    ///   Uses fabsf on fVal so the sign of Q doesn't appear in the label.
    void DrawSideLabel(HDC hdc, int x, int y, const wchar_t* szName, float fVal, const wchar_t* szUnit, COLORREF clr);

    void DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr);

    static const int MARGIN = 36;
};