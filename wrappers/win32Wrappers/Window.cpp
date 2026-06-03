/// @file Window.cpp
/// @brief Implementation of the window base class.

#include "Window.h"

Window::Window() : hinstance_app(NULL) {
  hwnd_self = NULL;
  arr_class_name[0] = L'\0';
}

Window::~Window() {}

BOOL Window::Create(HINSTANCE hInstance, const wchar_t* szTitle, int iWidth, int iHeight) {
  hinstance_app = hInstance;

  // ────── ⋆⋅☆⋅⋆ ────────
  // Generate a unique class name from this instance's pointer address.
  // Avoid collisions if multiple Window subclasses are instantiated.
  // ────── ⋆⋅☆⋅⋆ ────────
  wsprintf(arr_class_name, L"WndClass_%p", this);

  WNDCLASSEX wc       = {};
  wc.cbSize           = sizeof(WNDCLASSEX);
  wc.lpfnWndProc      = Window::WndProc; // Static trampoline
  wc.hInstance        = hInstance;
  wc.lpszClassName    = arr_class_name;
  wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);

  if(!RegisterClassEx(&wc)) return FALSE;

  // Pass 'this' as the last parameter. It arrives in WM_NCCREATE via
  // CREATESTRUCT::lpCreateParams where the trampoline picks it up
  hwnd_self = CreateWindowEx(
    0, arr_class_name, szTitle,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    iWidth, iHeight,
    NULL, NULL, hInstance,
    this // <- Our this pointer
  );

  return (hwnd_self != NULL);
}

void Window::Show(int nCmdShow) {
  if(hwnd_self) {
    ShowWindow(hwnd_self, nCmdShow);
    UpdateWindow(hwnd_self);
  }
}

int Window::Run() {
  MSG msg_current = {};
  while(GetMessage(&msg_current, NULL, 0, 0)) {
    TranslateMessage(&msg_current);
    DispatchMessage(&msg_current);
  }
  return (int)msg_current.wParam;
}

LRESULT Window::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    case WM_CREATE:
      OnCreate();
      return 0;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd_self, &ps);
      OnPaint(hdc);
      EndPaint(hwnd_self, &ps);
      return 0;
    }

    case WM_COMMAND:
      OnCommand(LOWORD(wParam), HIWORD(wParam));
      return 0;

    case WM_TIMER:
      OnTimer((int)wParam);
      return 0;

    case WM_DESTROY:
      OnDestroy();
      PostQuitMessage(0);
      return 0;

    case WM_HSCROLL:
      OnScroll((HWND)lParam, (int)LOWORD(wParam));
      return 0;
  }

  return DefWindowProc(hwnd_self, uMsg, wParam, lParam);
}

// ────── ⋆⋅☆⋅⋆ ────────
//   Static trampoline
// ────── ⋆⋅☆⋅⋆ ────────
LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Window* pWindow = nullptr;

  if(uMsg == WM_NCCREATE) {
    // Finsh the this pointer out of the CREATESTRUCT
    CREATESTRUCT* pCs = reinterpret_cast<CREATESTRUCT*>(lParam);
    pWindow = reinterpret_cast<Window*>(pCs->lpCreateParams);

    // Store it in the window's user data slot for all future messages 
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));

    pWindow->hwnd_self = hwnd;
  } else {
    pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if(pWindow)
    return pWindow->OnMessage(uMsg, wParam, lParam);

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}