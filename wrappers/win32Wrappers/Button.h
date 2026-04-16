/// @file Button.h
/// @brief Wraps a Win32 BUTTON control (BS_PUSHBUTTON)

#pragma once
#include "UIElement.h"

class Button : public UIElement {
  public:
    /// @function Button
    /// @brief Creates and registers a pushbutton as a child of hwnd_parent
    /// @param hwnd_parant Parent window handle
    /// @param szLabel Button text
    /// @param iId Control ID used to identify WM_COMMAND events
    /// @param iX Left position relative to parent client area
    /// @param iY Top Position 
    /// @param iW Width in pixels
    /// @param iH Height in pixels
    Button(HWND hwnd_parent, const wchar_t* szLabel, int iId, int iX, int iY, int iW, int iH) {
      hwnd_self = CreateWindowEx(
        0, L"BUTTON", szLabel,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        iX, iY, iW, iH,
        hwnd_parent, (HMENU)(UINT_PTR)iId,
        NULL, NULL
      );
    }
};