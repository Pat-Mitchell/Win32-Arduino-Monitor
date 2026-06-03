/// @file Trackbar.h
/// @brief Wraps a Win32 msctls_trackbar32 (slider) control.
///   Position is read via TBM_GETPOS since trackbars don't use
///   GetWindowText. Range and tick interval are set at creation.

#include <commctrl.h> // Required for TRACKBAR_CLASS
#include "UIElement.h"

#pragma comment(lib, "comctl32.lib")
#pragma once

class Trackbar : public UIElement {
  public:
    /// @brief Cretes a horizontal slider with auto tick marks
    /// @param hwnd_parent Parent window handle
    /// @param iId         Control ID
    /// @param iX          Left position
    /// @param iY          Top Position
    /// @param iW          Width in pixels
    /// @param iH          Height in pixels
    /// @param iMin        Minimum value
    /// @param iMax        maximum value
    Trackbar(HWND hwnd_parent, int iId, int iX, int iY, int iW, int iH, int iMin, int iMax) {
      // TBS_HORZ      - Horizontal orientation
      // TBS_AUTOTICKS - Draws a tick mark at each step
      // TBS_TOOLTIPS  - Shows current value in a floating tooltip
      hwnd_self = CreateWindowEx(
        0, TRACKBAR_CLASS, L"Trackbar Control",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | TBS_TOOLTIPS,
        iX, iY, iW, iH,
        hwnd_parent, (HMENU)(UINT_PTR)iId,
        NULL, NULL
      );

      // TBM_SETRANGE - wParam TRUE forces immediate redraw
      // TBM_SETTICFREQ - One tick per unit
      SendMessage(hwnd_self, TBM_SETRANGE, TRUE, MAKELPARAM(iMin, iMax));
      SendMessage(hwnd_self, TBM_SETTICFREQ, 10, 0); // Tick every 10 units
      SendMessage(hwnd_self, TBM_SETPOS, TRUE, iMin);
    }

    /// @brief Returns the current slider position
    /// @return int position in the range [iMin, iMax]
    int GetPos() const {
      if(!hwnd_self) return 0;
      return (int)SendMessage(hwnd_self, TBM_GETPOS, 0, 0);
    }

    /// @brief Sets the slider to a specific position
    /// @param iPos Target position (Clamped to range by Win32)
    void SetPos(int iPos) {
      if(hwnd_self) SendMessage(hwnd_self, TBM_SETPOS, TRUE, (LPARAM)iPos);
    }

    /// @brief Sets how often tick marks are drawn
    /// @param iFreq One tick every iFreq units
    void SetTickFreq(int iFreq) {
      if(hwnd_self) SendMessage(hwnd_self, TBM_SETTICFREQ, (WPARAM)iFreq, 0);
    }
};