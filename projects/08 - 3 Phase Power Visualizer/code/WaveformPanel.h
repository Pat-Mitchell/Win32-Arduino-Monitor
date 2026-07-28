/// @file WaveformPanel.h
/// @brief GCI panel that plots all three phase voltage waveforms across
///   Once full AC cycle (0 to 360 deg). Reads phase offsets directly
///   from ThreePhaseState

#pragma once
#include <windows.h>
#include <cmath>
#include "ThreePhaseMath.h"

class WaveformPanel {
  public:
    void Draw(HDC hdc, const RECT& rc, const ThreePhaseState& s);

  private:
    /// @brief Draws the horizontal zero line, vertical grid lines at
    ///   0/120/240/360 degrees and labels below the zero lines
    void DrawAxes(HDC hdc, const RECT& rc, int ox, int oy, int iAvailW);

    /// @brief Plots V = fPeak * cos(theta + fOffset) as a polyline across
    ///   one full cycle. Theta maps linearly from 0 to 2pi across iAvailW
    void DrawWave(HDC hdc, int ox, int oy, int iAvailW, float fPeak, float fScale, float fOffset_rad, COLORREF clr);

    /// @brief Places a wave label just above the panel top at the wave's
    ///   peak x-position. Peak position is derived from the offset so
    ///   labels are naturally separated for all valid offset combinations.
    void DrawWaveLabel(HDC hdc, int ox, int oy, int iAvailW, float fPeak, float fScale, float fOffset_rad, const wchar_t* sz, COLORREF clr);

    void DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr);

    static const int MARGIN = 28;
    static const int STEPS = 360;
};