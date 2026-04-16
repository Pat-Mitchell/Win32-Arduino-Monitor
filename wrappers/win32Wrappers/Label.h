/// @file Label.h
/// @brief Wraps a Win32 STATIC control for noninteractive display text

#pragma once
#include "UIElement.h"

class Label : public UIElement {
  public:

    /// @function Label
    /// @brief Creates a left-aligned static text label
    /// @param hwnd_parent Parent window handle
    /// @param szText Initial label text
    /// @param iX Left position
    /// @param iY Top position
    /// @param iW Width in pixels
    /// @param iH Height in pixels
    Label(HWND hwnd_parent, const wchar_t* szText, int iX, int iY, int iW, int iH) {
      hwnd_self = CreateWindowEx(
        0, L"STATIC", szText,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        iX, iY, iW, iH,
        hwnd_parent, NULL,
        NULL, NULL
      );
    }
};