/// @file win32_drawing.cpp
/// @brief GDI text and shape drawing

#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  const wchar_t* szClassName = L"DrawingWindow";

  WNDCLASSEX wc       = {};
  wc.cbSize           = sizeof(WNDCLASSEX);
  wc.lpfnWndProc      = WndProc;
  wc.hInstance        = hInstance;
  wc.lpszClassName    = szClassName;
  wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
  RegisterClassEx(&wc);

  HWND hwnd_main = CreateWindowEx(
    0, szClassName, L"Win32 Lesson 02 - Drawing",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    800, 600, NULL, NULL, hInstance, NULL
  );

  ShowWindow(hwnd_main, nCmdShow);
  UpdateWindow(hwnd_main);

  MSG msg_current = {};
  while(GetMessage(&msg_current, NULL, 0, 0)) {
    TranslateMessage(&msg_current);
    DispatchMessage(&msg_current);
  }
  return (int)msg_current.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      // ────── ⋆⋅☆⋅⋆ ────────
      // Drawing Text
      // SetTextColor sets the font color.
      // SetBkMode(TRANSPARENT) stops GDI from painting a box behind text.
      // DrawText handles wrapping and alignment inside a RECT.
      // TextOut is simpler. Just an x,y position, no layout options.
      // ────── ⋆⋅☆⋅⋆ ────────
      SetTextColor(hdc, RGB(30, 30, 30));
      SetBkMode(hdc, TRANSPARENT);

      // TextOut - simple x,y placement
      TextOut(hdc, 20, 20, L"Hello from GDI!", 15);

      // DrawText - draws within a bounding rect with alignment flags
      RECT rect_label = { 20, 50, 400, 80 };
      DrawText(hdc, L"DrawText with DT_LEFT alignment",
               -1, // -1 means the string is null-terminated
               &rect_label,
               DT_LEFT | DT_VCENTER | DT_SINGLELINE
      );

      // ────── ⋆⋅☆⋅⋆ ────────
      // CUSTOM FONT
      // CreateFont gives you control over size, weight, and face name.
      // Always DeleteObject() and GDI object you create - they leak
      // if you don't. SelectObject swaps the new font in and returns
      // the old one so you can restor it afterward.
      // ────── ⋆⋅☆⋅⋆ ────────
      HFONT hfont_large = CreateFont(
        36,             // Height in logical units
        0,              // Width - 0 lets GDI pick to match height
        0,              // Escapement (rotation of text run), in tenths of degrees
        0,              // Orientation (rotation of each char), in tenths of degrees
        FW_BOLD,        // Weight: FW_NORMAL, FW_BOLD, FW_THIN, etc.
        FALSE,          // Italic
        FALSE,          // Underline
        FALSE,          // Strikeout
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, // Rendering quality - CLEARTYPE looks best on LCD
        DEFAULT_PITCH,
        L"Comic Sans MS" // Dont face name
      );

      HFONT hfont_old = (HFONT)SelectObject(hdc, hfont_large);
      SetTextColor(hdc, RGB(0, 80, 200));
      wchar_t myWString[] = L"Bold Comic Sans MS, size 36";
      TextOut(hdc, 20, 100, myWString, wcslen(myWString));

      SelectObject(hdc, hfont_old); // Restore original font
      DeleteObject(hfont_large);    // Free our font

      // ────── ⋆⋅☆⋅⋆ ────────
      // Drawing Shapes
      // Every Shape uses the currently selected Pen (outline) and
      // Brush (fill). GDI has stock objects for common cases, or you
      // can create custom ones with CreatePen / CreateSolidBrush.
      // ────── ⋆⋅☆⋅⋆ ────────

      // --- Rectangle ---
      // CreatePen(style, width, color): PS_SOLID, PS_DASH, PS_DOT, etc.
      HPEN hpen_blue = CreatePen(PS_SOLID, 2, RGB(0, 80, 200));
      HBRUSH hbrush_lt_blue = CreateSolidBrush(RGB(173, 216, 230));

      HPEN hpen_old = (HPEN)SelectObject(hdc, hpen_blue);
      HBRUSH hbrush_old = (HBRUSH)SelectObject(hdc, hbrush_lt_blue);

      Rectangle(hdc, 20, 180, 220, 300); // Left, top, right, bottom

      // --- Ellipse ---
      HPEN hpen_red = CreatePen(PS_SOLID, 2, RGB(180, 0, 0));
      HBRUSH hbrush_pink = CreateSolidBrush(RGB(255, 182, 193));

      SelectObject(hdc, hpen_red);
      SelectObject(hdc, hbrush_pink);

      Ellipse(hdc, 240, 180, 440, 300);

      // -- Line ---
      // MoveToEx sets the start point; LineTo draws to the end point.
      HPEN hpen_green = CreatePen(PS_SOLID, 3, RGB(0, 140, 0));
      SelectObject(hdc, hpen_green);

      MoveToEx(hdc, 460, 180, NULL); // NULL = Don't need the old position back
      LineTo(hdc, 620, 300);

      // Restore original pen and brush before cleanup
      SelectObject(hdc, hpen_old);
      SelectObject(hdc, hbrush_old);

      // Always delete every GDI object before cleanup
      // GDI objects live in a shared kernel pool
      // Leaking them degrades the whole OS
      DeleteObject(hpen_blue);
      DeleteObject(hbrush_lt_blue);
      DeleteObject(hpen_red);
      DeleteObject(hbrush_pink);
      DeleteObject(hpen_green);

      EndPaint(hwnd, &ps);

      return 0;
    }

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}