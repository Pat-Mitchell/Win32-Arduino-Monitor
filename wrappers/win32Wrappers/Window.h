/// @file Window.h
/// @brief Concrete base class for top level Win32 windows.
///        Owns the window class registration, message pump, and the
///        WndProc trampoline that bridges the C call back into virtual methods.

#pragma once
#include "UIElement.h"

class Window : public UIElement {
  public:

    Window();
    virtual ~Window();

    /// @function Create
    /// @brief Registers the window class and creates the Win32 window.
    /// @param Hinstance App instance handle from WinMain.
    /// @param szTitle Tile bar text.
    /// @param iWidth Window width in pixels
    /// @param iHeight Window height in pixels
    /// @return TRUE on success, FALSE on failure
    BOOL Create(HINSTANCE hInstance, const wchar_t* szTitle, int iWidth, int iHeight);

    /// @function Show
    /// @brief Makes the window visible
    /// @param nCmdShow Show state passed in from WinMain
    /// @return void
    void Show(int nCmdShow);

    /// @function Run
    /// @brief Runs the message pump until WM_QUIT is received
    /// @return Exit code from WM_QUIT
    int Run();

  protected:
    // ────── ⋆⋅☆⋅⋆ ────────
    // Virtual message handlers
    // Override these in app subclass.
    // Default implementation call DefWindowProc or do nothing
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function OnCreate
    /// @brief Called when WM_CREATE fires. Create Child controls here.
    virtual void OnCreate() {}

    /// @function OnDestroy
    /// @brief Called when WM_DESTROY fires. Clean up resources
    virtual void OnDestroy() {}

    /// @function OnPaint
    /// @brief Called when WM_PAINT fires. HDC is alread acquired
    /// @param hdc Device context for drawing
    virtual void OnPaint(HDC hdc) {}

    /// @function OnCommand
    /// @brief Called when WM_COMMAND fires
    /// @param iControlId The ID of the control that fired
    /// @param iNotifCode Notification code (BN_CLICKED, EN_CHANGE, etc.)
    virtual void OnCommand(int iControlId, int iNotifCode) {}

    /// @function OnTimer
    /// @brief Called when WM_TIMER fires.
    /// @param iTimerId ID passed to SetTimer.
    virtual void OnTimer(int iTimerId) {}

    /// @function OnMessage
    /// @brief CatchAll for messages not covered by the named virtuals
    ///        Override for custom message handling beyonf the common set.
    /// @param uMsg Message ID
    /// @param wParam Message data
    /// @param lParam Message data
    /// @return Result passed back to the OS
    virtual LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// @function OnScroll
    /// @brief Called when WM_HSCROLL fires (trackbar or scrollbar moved).
    /// @param hwnd_control Handle to the control that fired
    /// @param iCode Scroll code (TB_THUMBTRACK, TB_ENDTRACK, etc.)
    virtual void OnScroll(HWND hwnd_control, int iCode) {}

    HINSTANCE hinstance_app;
  
  private:
    // ────── ⋆⋅☆⋅⋆ ────────
    // WndProc trampoline
    // Static so the OS can call it as a C function
    // Retreives the Windows* stored in GWLP_USERDATA and forwards to OnMessage
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @function WndProc
    /// @brief Static C-Style callback required by Win32. Forwards to OnMessage
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// @brief Unique class name per window instance (uses pointer address)
    wchar_t arr_class_name[64];
};