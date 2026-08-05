/// @file MotorPanel.h
/// @brief GDI panel that renders a bidrectional motor speed visualizer
///   Draws a vertical bar with zero at center.
///   Fill goes up for forward and down for reverse.
///   Includes a numeric speed readout.

#pragma once
#include <windows.h>

class MotorPanel {
  public:
    /// @param szLabel Label displayed above the bar ("MOTOR 1")
    MotorPanel(RECT rcPanel, const wchar_t* szLabel);
    void Draw(HDC hdc, int iSpeed) const;

  private:
    RECT rc_panel;
    wchar_t arr_label[32];
};