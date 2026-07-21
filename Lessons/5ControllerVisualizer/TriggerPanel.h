/// @file TriggerPanel.h
/// @brief GDI panel that renders a trigger axis visualizer.
///   Draws a horizontal filled bar scaled to the trggier value,
///   a label, and a numeric readout.

#pragma once
#include <windows.h>

class TriggerPanel {
  public:
    /// @param rcPanel Bouding rect in parent client rect
    /// @param szAxisPrefix Axis prefix for the readout label (L"L" -> Left trigger)
    TriggerPanel(RECT rcPanel, const wchar_t* szAxisPrefix);

    /// @brief Renders the full panel into hdc
    ///   Call from WM_PAINT afer BeginPaint
    /// @param hdc Device context to draw into
    /// @param bValue Raw trigger value from XInput [0, 255]
    ///   0 = bar empty, 255 = bar fully filled
    void Draw(HDC hdc, BYTE bValue) const;

  private:
    RECT rc_panel; // Bounding rect of the whole panel
    wchar_t arr_axis_prefix[4]; // Axis prefix (L"L", or L"R")
};