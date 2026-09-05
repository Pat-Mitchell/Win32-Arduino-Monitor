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
}