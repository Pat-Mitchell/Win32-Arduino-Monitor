/// @file ProgressBar.h
/// @brief Wraps a Win32 PROGRESS_CLASS control
///   Requires commctrl.h and comctl32.lib

#pragma once
#include <commctrl.h>
#include "UIElement.h"

class ProgressBar : public UIElement {
  public:
    /// @brief Creates a horizontal progress bar
    /// @param hwnd_parent Parent window handle
    /// @param iId Control ID
    /// @param iX Left position
    /// @param iY Top position
    /// @param iW Width in pixels
    /// @param iH Height in pixels
    ProgressBar(HWND hwnd_parent, int iId, int iX, int iY, int iW, int iH) {
      hwnd_self = CreateWindowEx( 0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, iX, iY, iW, iH, hwnd_parent, (HMENU)(UINT_PTR)iId, NULL, NULL);
      
      SetRange(0, 100);
      SetPos(0);
    }

    /// @brief Sets the minimum and maximum values
    /// @param iMin minimum value
    /// @param iMax maximum value
    void SetRange(int iMin, int iMax) {
      if(hwnd_self) {
        SendMessage(hwnd_self, PBM_SETRANGE, 0, MAKELPARAM(iMin, iMax));
      }
    }

    /// @brief Sets the current position
    /// @param iPos Position within [iMin, iMax]
    void SetPos(int iPos) {
      if(hwnd_self) {
        SendMessage(hwnd_self, PBM_SETPOS, (WPARAM)iPos, 0);
      }
    }

    /// @brief Sets the step increment used by StepIt
    /// @param iStep Amount to advance per StepIt call
    void SetStep(int iStep) {
      if(hwnd_self) {
        SendMessage(hwnd_self, PBM_SETSTEP, (WPARAM)iStep, 0);
      }
    }

    /// @brief Adnaves position by the current step increment
    void StepIt() {
      if(hwnd_self) {
        SendMessage(hwnd_self, PBM_STEPIT, 0, 0);
      }
    }
};