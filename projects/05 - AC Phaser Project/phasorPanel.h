/// @file phasorPanel.h
/// @brief GDI panel that draw the series RLC voltage phasor diagram
///   Uses the current-reference convention: I lies along the +X axis.
///   All phasor originate from a shard origin (left-center of the panel).

#pragma once 
#include "phasorMath.h"
#include <windows.h>

class PhasorPanel {
  public:
    void Draw(HDC hdc, const RECT& rc, const PhasorState& s);

  private:
    /// @brief Draws a line from (x0, y0) to (x1, y1)
    void DrawArrow(HDC hdc, int x0, int y0, int x1, int y1, COLORREF clr, bool bDashed = false);

    /// @brief Draw a polyline arc ar ARC_RADIUS from (i0x, i0y), sweeping from
    ///   0 radians to fPhi_rad. Used to annotate the pahse angle
    void DrawPhaseArc(HDC hdc, int cx, int cy, float fPhi_rad, COLORREF clr);

    /// @brief Renders transparent text at (x,y) in the given color.
    ///   Saves and restores HDC text color and background mode.
    void DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr);

    /// @brief Draws the horizontal and vertical axis lines through the origin.
    void DrawAxes(HDC hdc, const RECT& rc, int ox, int oy);

    static const int MARGIN = 24;
};