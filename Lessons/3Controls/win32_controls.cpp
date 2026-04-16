/// @file win32_controls.cpp
/// @brief Buttons, text inputs, and labels MW_COMMAND.

#include <windows.h>

// Control IDs. Arbitrary integers used to identify which control
// fired a WM_COMMAND. Define the as contants to avoid magic numbers
#define ID_BTN_SUBMIT 101
#define ID_BTN_CLEAR  102
#define ID_EDIT_INPUT 103
#define ID_STATIC_OUT 104

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  const wchar_t* szClassName = L"ControlsWindw";

  WNDCLASSEX wc       = {};
  wc.cbSize           = sizeof(WNDCLASSEX);
  wc.lpfnWndProc      = WndProc;
  wc.hInstance        = hInstance;
  wc.lpszClassName    = szClassName;
  wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
  RegisterClassEx(&wc);

  HWND hwnd_main = CreateWindowEx(
    0, szClassName, L"Wind32 Lesson 03 - Controls",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    500, 300, NULL, NULL, hInstance, NULL
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
  // Declared here so WM_COMMAND can access them.
  // static makes them persist between WndProc calls (they live for the
  // window's lifetime rather than being recreated each message).
  static HWND hwnd_edit_input = NULL;
  static HWND hwnd_static_out = NULL;

  switch(uMsg) {
    // WM_CREATE fires once, immediately after the window is created.
    // This is the right place to create all child controls.
    case WM_CREATE:
    {
      // --- Static label (read-only display text) ---
      // "STATIC" is a built-in Win32 class for non-interactice text.
      // SS_LEFT = left-align text within the control's rect.
      CreateWindowEx(
        0, L"STATIC", L"Enter some text:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 20, 200, 24,
        hwnd, // Parent window
        NULL, // No ID needed for purely decorative label
        NULL, NULL
      );

      // --- Single-line text input (EDIT control) ---
      // WS_BORDER draws the input box outline.
      // ES_AUTOHSCROLL scrolls horizontally when text exceeds the width.
      hwnd_edit_input = CreateWindowEx (
        0, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        20, 50, 300, 28,
        hwnd,
        (HMENU)ID_EDIT_INPUT, // Cast ID to HMENU - Wind32 quirk for controls
        NULL, NULL
      );

      // --- Buttons ---
      // "BUTTON" is the built-in class. BS_PUSHBUTTON = standard clickable button.
      CreateWindowEx(
        0, L"BUTTON", L"Submit",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 95, 100, 30,
        hwnd,
        (HMENU)ID_BTN_SUBMIT,
        NULL, NULL
      );

      CreateWindowEx(
        0, L"BUTTON", L"Clear",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, 95, 100, 30,
        hwnd,
        (HMENU)ID_BTN_CLEAR,
        NULL, NULL
      );

      // --- Output label (updated dynamically) ---
      hwnd_static_out = CreateWindowEx(
        0, L"STATIC", L"Output will appear here.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 145, 440, 24,
        hwnd,
        (HMENU)ID_STATIC_OUT,
        NULL, NULL
      );

      return 0;
    }

    // WM_COMAND fires when a button is clicked or an EDIT control
    // sends a notification. LOWORD(wParam) gives you the control ID.
    case WM_COMMAND:
    {
      int iControlId = LOWORD(wParam); // Which control fired
      int iNotifCode = HIWORD(wParam); // What it did (for EDIT controls)

      switch(iControlId)
      {
        case ID_BTN_SUBMIT:
        {
          // Read text from the EDIT control into a buffer.
          // GetWindowText works on any HWND, not just top-level windows.
          wchar_t arrBuf[256];
          GetWindowText(hwnd_edit_input, arrBuf, 256);

          // Build output string and push it to the static label.
          wchar_t arrOut[300];
          wsprintf(arrOut, L"You entered: %s", arrBuf);
          SetWindowText(hwnd_static_out, arrOut);
          return 0;
        }

        case ID_BTN_CLEAR:
        {
          SetWindowText(hwnd_edit_input, L"");
          SetWindowText(hwnd_static_out, L"Output till appear here.");
          SetFocus(hwnd_edit_input); // Return cursor focus to the input
          return 0;
        }
      }

      return 0;
    }

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}