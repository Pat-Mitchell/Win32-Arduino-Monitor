/// @file D2DWindow.cpp

#include "D2DWindow.h"

// Static members
ID2D1Factory* D2DWindow::s_pFactory = nullptr;
int D2DWindow::s_iRefCount = 0;

// App lifecycle
HRESULT D2DWindow::InitD2D() {
  if(s_pFactory) {
    return S_OK;
  }

  return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &s_pFactory);
}

void D2DWindow::ShutdownD2D() {
  if(s_pFactory) {
    s_pFactory->Release();
    s_pFactory = nullptr;
  }
}

// Constructor/Destructor
D2DWindow::D2DWindow()
  : pRT(nullptr)
  , pDWrite(nullptr)
  , pFmtSmall(nullptr)
  , pFmtMedium(nullptr)
  , pFmtTitle(nullptr)
{
  s_iRefCount++;
}

D2DWindow::~D2DWindow() {
  // Release text formats (device-independent. Survive device loss)
  if(pFmtTitle) {
    pFmtTitle->Release();
    pFmtTitle = nullptr;
  }

  if(pFmtMedium) {
    pFmtMedium->Release();
    pFmtMedium = nullptr;
  }

  if(pFmtSmall) {
    pFmtSmall->Release();
    pFmtSmall = nullptr;
  }

  if(pDWrite) {
    pDWrite->Release();
    pDWrite = nullptr;
  }  

  // Release device-dependent resources (render target)
  DiscardDeviceResources();

  // Release shared factory when last window closes
  if(--s_iRefCount == 0) {
    ShutdownD2D();
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// Resource management
// ────── ⋆⋅☆⋅⋆ ────────

HRESULT D2DWindow::CreateDeviceResources() {
  if(pRT) {
    return S_OK; // Already exists
  }

  if(!s_pFactory) {
    return E_FAIL; // InitD2D() not called
  }

  // Render target
  RECT rcClient;
  GetClientRect(hwnd_self, &rcClient);

  HRESULT hr = s_pFactory->CreateHwndRenderTarget(
    D2D1::RenderTargetProperties(),
    D2D1::HwndRenderTargetProperties(
      hwnd_self,
      D2D1::SizeU(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top)
    ),
    &pRT
  );

  if(FAILED(hr)) {
    return hr;
  }

  // Anti-aliased geometry. Diagonallines and curves are smooth.
  pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

  // ClearType text anti-aliasing via DirectWrite
  pRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

  // DirectWrite factory and text formats
  // Device-independent. Create once, survive device loss.
  // Duard with nullptr check so they survive render target recreation
  if(!pDWrite) {
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWrite));
    if(FAILED(hr)) {
      return hr;
    }

    // Small. 11px, normal weight. 
    pDWrite->CreateTextFormat(
      L"Segoe UI", nullptr,
      DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      11.0f, L"en-us", &pFmtSmall
    );

    // Medium. 13px, normal wight
    pDWrite->CreateTextFormat(
      L"Segoe UI", nullptr,
      DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      13.0f, L"en-us", &pFmtMedium
    );

    // Title. 16px, semi-bold
    pDWrite->CreateTextFormat(
      L"Segoe UI", nullptr,
      DWRITE_FONT_WEIGHT_SEMI_BOLD,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      16.0f, L"en-us", &pFmtTitle
    );
  }

  return S_OK;
}

void D2DWindow::DiscardDeviceResources() {
  if(pRT) {
    pRT->Release();
    pRT = nullptr;
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// Message handling
// ────── ⋆⋅☆⋅⋆ ────────

LRESULT D2DWindow::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    // WM_CREATE
    // Create D2D resources first so pRT is valid when OnCreate() runs.
    // Subclasses can us pRT inside their OnCreate() if needed.
    case WM_CREATE:
      CreateDeviceResources();
      OnCreate();
      return 0;

    // WM_ERASEBKGND
    // Suppress GDI background erase. D@D clears the background in
    // BeginDraw. Returning 1 prevents the white flash on resize.
    case WM_ERASEBKGND:
      return 1;

    // WM_PAINT
    // D2D draw cycle. BeginPaint/EndPaint still required to validate the dirty
    // region with the OS. Without them, WM_PAINT fires continuously.
    // The HDC from BeginPaint is unused (D2D draws to its own surface).
    case WM_PAINT: {
      PAINTSTRUCT ps;
      BeginPaint(hwnd_self, &ps);

      if(SUCCEEDED(CreateDeviceResources()) && pRT) {
        pRT->BeginDraw();
        pRT->SetTransform(D2D1::Matrix3x2F::Identity());

        OnD2DPaint(pRT);

        HRESULT hr = pRT->EndDraw();

        // D2DERR_RECREATE_TARGET: GPU device was lost.
        // Triggers on remote desktop connect, driver update, etc.
        // Discard resources and schedule a repaint. Next WM_PAINT
        // will call CreateDeviceResources() and recover automatically.
        if(hr == D2DERR_RECREATE_TARGET) {
          DiscardDeviceResources();
          InvalidateRect(hwnd_self, NULL, FALSE);
        }
      }
      EndPaint(hwnd_self, &ps);
      return 0;
    }

    // WM_SIZE
    // Resize the render target to match the new client area.
    // ID2D1HwndRenderTarget::Resize() is lightweight. No recreation needed.
    case WM_SIZE: {
      if(pRT) {
        pRT->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
      }
      OnSize(LOWORD(lParam), HIWORD(lParam));
      InvalidateRect(hwnd_self, NULL, FALSE);
      return 0;
    }
  }

  // Forward everything else (WM_COMMAND, WM_TIMER, WM_HSCROLL, WM_DESTROY...)
  // to the existing Window base class handler unchanged
  return Window::OnMessage(uMsg, wParam, lParam);
}

// ────── ⋆⋅☆⋅⋆ ────────
// Utilities
// ────── ⋆⋅☆⋅⋆ ────────

D2D1::ColorF D2DWindow::ColorF(COLORREF clr, float fAlpha) {
  return D2D1::ColorF(
    GetRValue(clr) / 255.0f,
    GetGValue(clr) / 255.0f,
    GetBValue(clr) / 255.0f,
    fAlpha
  );
}

ID2D1SolidColorBrush* D2DWindow::CreateBrush(D2D1::ColorF color) const {
  if(!pRT) {
    return nullptr;
  }

  ID2D1SolidColorBrush* pBrush = nullptr;
  pRT->CreateSolidColorBrush(color, &pBrush);
  return pBrush;
}

void D2DWindow::DrawTextAt(ID2D1HwndRenderTarget* pRT, const wchar_t* szText, IDWriteTextFormat* pFmt, D2D1::ColorF clr, D2D1_RECT_F rcLayout, DWRITE_TEXT_ALIGNMENT eAlign) const {
  if(!szText || !pFmt || !pRT) {
    return;
  }

  // Text alignment is a property of the format. set and restore
  pFmt->SetTextAlignment(eAlign);
  pFmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  ID2D1SolidColorBrush* pBrush = nullptr;
  if(SUCCEEDED(pRT->CreateSolidColorBrush(clr, &pBrush))) {
    pRT->DrawText(szText, static_cast<UINT32>(wcslen(szText)), pFmt, rcLayout,pBrush);
    pBrush->Release();
  }
}