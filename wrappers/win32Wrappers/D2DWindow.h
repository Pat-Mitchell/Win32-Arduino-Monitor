/// @file D2DWindow.h
/// @brief Extends Window with Direct2D rendering infrastructure.

#pragma once
#include "Window.h"
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

class D2DWindow : public Window {
  public:
    D2DWindow();
    virtual ~D2DWindow();

    // ────── ⋆⋅☆⋅⋆ ────────
    //    App lifecycle
    // Call once from WinMain
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Creates the shared ID2D1Factory. Call before and D2DWindow::Create().
    ///   Equivalent to InitCommonControlsEx. One-time per process setup.
    /// @return S_OK on success
    static HRESULT InitD2D();

    /// @brief Releases the shared factory. Called automaticall when the last
    ///   D2DWindow is destroyed. Explicit call from WinMain is optional.
    static void ShutdownD2D();

  protected:
    // ────── ⋆⋅☆⋅⋆ ────────
    // Override this instead of OnPaint
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Called each frame with a valid, begun render target.
    ///   BeginDraw has Already been called. Do NOT call BeginDraw/EndDraw
    ///   yourself. The base class manages the draw cycle and device loss.
    /// @param pRT 
    virtual void OnD2DPaint(ID2D1HwndRenderTarget* pRT) {}

    /// @brief Called when the window is resized. The render target has already
    ///   been resized. Override to recalculate panel rects.
    /// @param iW New client width in pixels
    /// @param iH New client height in pixels
    virtual void OnSize(UINT iW, UINT iH) {}

    // ────── ⋆⋅☆⋅⋆ ────────
    // Message handling
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Intercepts WM_CREATE, WM_PAINT, WM_SIZE, WM_ERASEBKGND.
    ///   All other messages forwarded to Window::OnMessage
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    // ────── ⋆⋅☆⋅⋆ ────────
    // Resource management
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Creates the HWND render target and DirectWrite formats.
    ///   Called automatically at WM_CREATE and after device loss.
    ///   Safe to call if resources already exist (no-op)
    HRESULT CreateDeviceResources();

    /// @brief Releases device-dependent resources. Override in subclasses to
    ///   also release any cached brushes or bitmaps they own.
    ///   Always call the base version from overrides.
    virtual void DiscardDeviceResources();

    // ────── ⋆⋅☆⋅⋆ ────────
    // Drawing utilities
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Converts a Win32 COLORREF to a D2D1::ColorF.
    ///   Enables reuse of COLORREF constants from existing code
    /// @param clr GDI COLORREF value
    /// @param fAlpha Opacity 0.0 - 1.0. Default 1.0 = fully opaque
    /// @return 
    static D2D1::ColorF ColorF(COLORREF clr, float fAlpha = 1.0f);

    /// @brief Creates a solid color brush. Caller must Release().
    ///   Returns nullptr on failure. Always check before use.
    ///   For hot paint paths, cache brushes as members and recreate
    ///   them in CreateDeviceResources rather than calling this per frame.
    ID2D1SolidColorBrush* CreateBrush(D2D1::ColorF color) const;

    /// @brief Convenience wrapper around DrawText for a string in a rect
    /// @param pRT Render target
    /// @param szText Null-terminated wide string
    /// @param pFmt Text format (use pFmtSmall / pFmtMedium / pFmtTitle)
    /// @param clr Text color
    /// @param rcLayout Bounding rect for layout
    /// @param eAlign dwFlags. DWRITE_TEXT_ALIGNMENT flags. Default: Center
    void DrawTextAt(ID2D1HwndRenderTarget* pRT, const wchar_t* szText, IDWriteTextFormat* pFmt, D2D1::ColorF clr, D2D1_RECT_F rcLayout, DWRITE_TEXT_ALIGNMENT eAlign = DWRITE_TEXT_ALIGNMENT_CENTER) const;

    // ────── ⋆⋅☆⋅⋆ ────────
    // Protected members
    // ────── ⋆⋅☆⋅⋆ ────────

    /// @brief Active render target for this window instance.
    ///   Valid between CreateDeviceResources() and DiscardDeviceResources().
    ///   Check for nullptr before use in any code path that may fire before
    ///   WM_CREATE (e.g. timer callbacks before the window is shown)
    ID2D1HwndRenderTarget* pRT;

    /// @brief DirectWrite factory. Devie-independent, survives device loss
    IDWriteFactory* pDWrite;

    IDWriteTextFormat* pFmtSmall; // 11px Segoe UI
    IDWriteTextFormat* pFmtMedium; // 13px Segoe UI
    IDWriteTextFormat* pFmtTitle; // 16px Segoe UI SemiBold

    /// @brief Shared Direct2D factory. Device-independent. one per process
    //   Created by InitD2D(), released by ShutdownD2D().
    static ID2D1Factory* s_pFactory;

  private:
    static int s_iRefCount; // Live D2DWindow count. Drives ShutdownD2D
};