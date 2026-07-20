/// @file StickPanel.h
/// @brief GDI panel that renders a thumbstick visualizer.
///   Draws a circular field, a deadzone ring, a position dot,
///   and raw axis readouts.

#pragma once
#include <Windows.h>

class StickPanel {
  public:
    /// @brief Constructs a StickPanel occupying the given rect
    /// @param rcPanel Bounding rect in parent client coordinates
    /// @param szLabel Label string displayed above the circle
    /// @param szAxisPrefix Prefix used in axis readouts (L"L" for left, L"R" for right)
    StickPanel(RECT rcPanel, const wchar_t* szLabel, const wchar_t* szAxisPrefix);

    /// @brief Renders the full panel into hdc
    /// @param hdc Device context to draw into
    /// @param sFilteredX Deadzone-filtered X axis value used to position the dot
    /// @param sFilteredY Deadzone-filtered Y axis value used to position the dot
    /// @param sRawX Raw X axis value from XInput. Displayed in the readout below the circle
    /// @param sRawY Raw Y axis value from XInput. Displayed in the readout below the circle
    /// @param iDeadzone Deadzone radius in raw stick units. Controls the size of the deadzone ring drawn on the circle
    void Draw(HDC hdc, SHORT sFilteredX, SHORT sFilteredY, SHORT sRawX, SHORT sRawY, int iDeadzone) const;

  private:

    /// @brief Maps a filtered stick value pair to a pixel coordinate
    ///   within the circle, clamped to the circle's radius.
    /// @param sX Filtered X axis value
    /// @param sY Filtered Y axis value
    ///   @note XInput Y is up-positive. GDI Y is down-positive
    ///         This function inverts Y so up on stick = up on the screen
    /// @param ptCenter Center pixel of the circle
    /// @param iRadius Radius of the circle in pixels
    /// @param ptOut Receives the mapped pixel coordinate
    static void MapStickToCircle(SHORT sX, SHORT sY, POINT ptCenter, int iRadius, POINT& ptOut);

    RECT rc_panel; // Bounding rect of the whole panel
    wchar_t arr_label[32]; // Label drawn above the circle
    wchar_t arr_axis_prefix[4]; // Axis prefix for readouts
};

