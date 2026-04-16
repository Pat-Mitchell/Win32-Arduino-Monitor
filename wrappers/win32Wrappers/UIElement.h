/// @file UIElement.h
/// @brief Abstract base class for all Win32 UI controls.
///        Wraps a raw HWND and exposes common operations.

#pragma once
#include <windows.h>

class UIElement {
  public:
    /// @brief Default constructor
    UIElement() : hwnd_self(NULL) {}

    /// @brief Virtual destructor
    virtual ~UIElement() {}

    // ────── ⋆⋅☆⋅⋆ ────────
    //      Accessors
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function GetHandle
    /// @brief Returns the raw Win32 window handle
    /// @return HWND
    HWND GetHandle() const { return hwnd_self; }

    // ────── ⋆⋅☆⋅⋆ ────────
    //   Common operations
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function SetText
    /// @brief Sets the text content of the element
    /// @param szText Null terminated wide string
    /// @return void
    void SetText(const wchar_t* szText) {
      if(hwnd_self) SetWindowText(hwnd_self, szText);
    }

    /// @function GetText
    /// @brief Copies the element's current text into a caller supplied buffer.
    /// @param arrBuf Destionation buffer.
    /// @param iBufLen Size of the buffer in characters.
    /// @return void
    void GetText(wchar_t* arrBuf, int iBufLen) const {
      if(hwnd_self) GetWindowText(hwnd_self, arrBuf, iBufLen);
    }

    /// @function Enable
    /// @brief Enables user interaction with the element
    /// @return void
    void Enable() { if(hwnd_self) EnableWindow(hwnd_self, TRUE); }

    /// @function Disable
    /// @brief Disables user interaction and greys out the element
    /// @return void
    void Disable() { if(hwnd_self) EnableWindow(hwnd_self, FALSE); }

    /// @function Show
    /// @brief Makes the element visible.
    /// @return void
    void Show() { if(hwnd_self) ShowWindow(hwnd_self, SW_SHOW); }

    /// @function Hide
    /// @brief Hides the element with destroying it.
    /// @return void
    void Hide() { if(hwnd_self) ShowWindow(hwnd_self, SW_HIDE); }

  protected:
    /// @brief The underlying Win32 handle. Set by subclasses after creation.
    HWND hwnd_self;
};