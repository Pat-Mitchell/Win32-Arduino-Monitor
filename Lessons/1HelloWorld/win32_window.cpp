/// @file win32_window.cpp
/// @brief A minimal Win32 window with a message pump.
/// g++ win32_window.cpp -o win32_window -mwindows -DUNICODE -D_UNICODE

#include <windows.h>

// Forward declaration
// WinMain references WndProc before it's defined.
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/// @function WinMain
/// @description Application entry point.
///              Registers the window class
///              Creates the window
///              runs the message pump
/// @param hInstance     Handle to thie running instance of the app.
/// @param hPrevInstance Legacy - always NULL
/// @param lpCmdLine     Command-line string
/// @param nCmdShow      Initial window state (maximized, normal, etc.)
/// @return Exit code from the message pump.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // ────── ⋆⋅☆⋅⋆ ────────
  // 1. Register a Window Class
  // A "class" here is not a C++ class. It's a Win32 template that
  // describes a category of window (its icon, cursor, background, and
  // crucially, which WndProc handles its messages).
  // ────── ⋆⋅☆⋅⋆ ────────
  const wchar_t* szClassName = L"MainWindowClass";

  WNDCLASSEX wc      = {}; // Zero-initialize all fields
  wc.cbSize          = sizeof(WNDCLASSEX);
  wc.lpfnWndProc     = WndProc; // Pointer to our message handler
  wc.hInstance       = hInstance;
  wc.lpszClassName   = szClassName;
  wc.hCursor         = LoadCursor(NULL, IDC_ARROW); // Standard Arrow
  wc.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1); // Default white bg
  // wc.hbrBackground   = CreateSolidBrush(RGB(30, 30, 30)); // dark grey bg

  RegisterClassEx(&wc);

    // ────── ⋆⋅☆⋅⋆ ────────
    // 2. Create The Window
    // CreateWindowEx instantiates a window from the class above.
    // It returns an HWND. A "handle to a window". This opque ID is how
    // you reference any window in subsequent Win32 calls.
    // ────── ⋆⋅☆⋅⋆ ────────
    HWND hwnd_main = CreateWindowEx(
      0,                    // Optional extended styles
      szClassName,          // Which registered class to use
      L"Win32 Lesson 1",    // Title bar text
      WS_OVERLAPPEDWINDOW,  // Style (standard resizable window)
      CW_USEDEFAULT,        // X Position
      CW_USEDEFAULT,        // Y Position
      800, 600,             // Width, height in pixels
      NULL,                 // Parent window (None. This is top-level)
      NULL,                 // Menu (none)
      hInstance,
      NULL                  // Extra data passed to WndProc (none yet)
    );

    if(hwnd_main == NULL) return -1; // CreateWindowEx failed

    // ────── ⋆⋅☆⋅⋆ ────────
    // 3. Show the window
    // The eindow exists but is invisible until you call these two.
    // ShowWindow sets visibility; UpdateWindow forces an immediate WM_PAINT.
    // ────── ⋆⋅☆⋅⋆ ────────
    ShowWindow(hwnd_main, nCmdShow);
    UpdateWindow(hwnd_main);

    // ────── ⋆⋅☆⋅⋆ ────────
    // 4. The Message pump
    // GetMessage blocks until a message arrives, the fills the MSG struct.
    // It returns FALSE when it pulls WM_QUIT (that's the exit signal).
    // TranslateMessage handles keyboard input (raw Keys -> WM_CHAR events).
    // DispatchMessage routes the message to the orrect WndProc.
    // ────── ⋆⋅☆⋅⋆ ────────
    MSG msg_current = {};
    while(GetMessage(&msg_current, NULL, 0, 0)) {
      TranslateMessage(&msg_current);
      DispatchMessage(&msg_current);
    }

    return (int)msg_current.wParam; // Exit code from WM_QUIT
}

/// @function WndProc
/// @description Message handler (callback) for the main window. Windows calls this
///              every time an event occurs. uMsg identifies the event type.
/// @param hwnd Handle to the window receiving the message.
/// @param uMsg The message ID (e.g. WM_PAINT, WM_DESTROY).
/// @param wParam Message-specific data (meaning varies per message).
/// @param lParam Message-specific data (meaning varies per message).
/// @return 0 if the message was handled; DefWindowProc for everything else.
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    // WM_PAINT
    // The window (or part of it) needs to be redrawn.
    // Every visible window must handle this or it will show garbage.
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps); // Aquire the device context

      // For now, just let the default background fill happen.
      // We'll draw things here in later.

      EndPaint(hwnd, &ps); // Release the device context
      return 0;
    }

    // WM_DESTROY
    // The user closed the window.
    // PostQuitMessage puts WM_QUIT on the queue, which ends the pump loop.
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  // Any message we don't explicitly handle gets passed to the default
  // handler. This is mandatory. Without it, basic behavior like
  // resizing, minimizing, and system menus will break.
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}