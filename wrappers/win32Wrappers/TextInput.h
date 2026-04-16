/// @file TextInput.h
/// @brief Wraps a Win32 EDIT control. Supports both single-line input
///        and multiline read-only output display

#pragma once
#include "UIElement.h"

class TextInput : public UIElement {
  public:
    /// @function TextInput
    /// @brief Creates an EDIT control
    /// @param hwnd_parent Parent window handle.
    /// @param iId Control ID
    /// @param iX Left position
    /// @param iY Top position
    /// @param iW Width in pixels
    /// @param iH Height in pixels
    /// @param bMultiline TRUE for multiline output box, FALSE for single line input
    TextInput(HWND hwnd_parent, int iId, int iX, int iY, int iW, int iH, BOOL bMultiline = FALSE) {
      DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;

      if(bMultiline) {
        dwStyle =WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL;
      }

      hwnd_self = CreateWindowEx(
        bMultiline ? WS_EX_CLIENTEDGE : 0,
        L"EDIT", NULL,
        dwStyle,
        iX, iY, iW, iH,
        hwnd_parent, (HMENU)(UINT_PTR)iId,
        NULL, NULL
      );
    }

    /// @function SetReadOnly
    /// @brief Toggles whether the user can type into the control
    /// @param bReadOnly TRUE to lock, FALSE to allow editing
    /// @return void
    void SetReadOnly(BOOL bReadOnly) {
      if(hwnd_self)
        SendMessage(hwnd_self, EM_SETREADONLY, bReadOnly, 0);
    }

    /// @function Append
    /// @brief Appends text to the end of the control and scrolls to bottom
    ///        Intended for multiline output displays
    /// @param szText text to append
    /// @return void
    void Append(const wchar_t* szText) {
      if(!hwnd_self) return;

      int iLen = GetWindowTextLength(hwnd_self);
      SendMessage(hwnd_self, EM_SETSEL, iLen, iLen);
      SendMessage(hwnd_self, EM_REPLACESEL, FALSE, (LPARAM)szText);
      SendMessage(hwnd_self, EM_SCROLLCARET, 0, 0);
    }

    /// @function Clear
    /// @brief Clears all text from the control
    /// @return void
    void Clear() {
      if(hwnd_self) SetWindowText(hwnd_self, L"");
    }
};