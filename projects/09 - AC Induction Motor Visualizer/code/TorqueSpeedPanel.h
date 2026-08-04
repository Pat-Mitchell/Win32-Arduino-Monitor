/// @file TorqueSpeedPanel.h
/// @brief GDI panel that plots the normalized torque-speed curve for a 
///   three-phase induction motor. Key operating points are annotated
///   with colored dots and labels. Curve redraws on input change only.

#pragma once
#include <windows.h>
#include <cmath>
#include <cwchar>
#include "MotorMath.h"
#include "..\..\..\wrappers\win32Wrappers\PlotPanel.h"

class TorqueSpeedPanel {
  public:
    void Draw(HDC hdc, const RECT& rc, const MotorState& s);

  private:
    void DrawNsMarker (HDC hdc, const RECT& rcPlot);
    void DrawKeyPoints (HDC hdc, const RECT& rcPlot, const MotorState& s);
    void DrawDot(HDC hdc, int x, int y, int iR, COLORREF clr);
    void DrawLabel(HDC hdc, int x, int y, const wchar_t* sz, COLORREF clr);


    /// @brief Maps rotor speed (x) and normalized torque (y)
    static int MapX(float fVal, float fMin, float fMax, const RECT& rcPlot);
    static int MapY(float fVal, float fMin, float fMax, const RECT& rcPlot);

    // Y axis ceiling. 1.15 gives 15% headroom
    static constexpr float Y_MAX = 1.15f;
};