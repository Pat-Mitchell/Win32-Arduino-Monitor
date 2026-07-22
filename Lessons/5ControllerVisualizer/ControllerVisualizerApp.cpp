/// @file ControllerVisualizerApp.cpp
/// @brief Implementation of the Controller Input visualizer application

#include "ControllerVisualizerApp.h"

// ────── ⋆⋅☆⋅⋆ ────────
// GDI colot palette
// ────── ⋆⋅☆⋅⋆ ────────
namespace {
  const COLORREF CLR_APP_BG = RGB(30, 30, 30);
  const COLORREF CLR_BTN_ACTIVE = RGB(50, 200, 100);
  const COLORREF CLR_BTN_INACTIVE = RGB(50, 50, 50);
  const COLORREF CLR_BTN_BORDER = RGB(100, 100, 100);
  const COLORREF CLR_BTN_TEXT = RGB(200, 200, 200);
}

// ────── ⋆⋅☆⋅⋆ ────────
// Layout constants (in pixels)
// ────── ⋆⋅☆⋅⋆ ────────
namespace Layout {
  // window
  const int WIN_W = 800;
  const int WIN_H = 500;

  // Control strip (top bar)
  const int STRIP_H = 36;
  const int STRIP_PAD = 8;

  // GDI panels below the strip
  const int PANEL_TOP = STRIP_H + 4;
  const int PANEL_H = 340;

  // Left stick panel
  const int LSTICK_X = 0;
  const int LSTICK_W = 220;

  // Center column (triggers + buttons)
  const int CENTER_X = LSTICK_W;
  const int CENTER_W = 360;

  // RIGHT stick panel
  const int RSTICK_X = LSTICK_W + CENTER_W;
  const int RSTICK_W = WIN_W - RSTICK_X;

  // Trigger panels inside the center column
  const int TRIG_PAD = 8;
  const int TRIG_H = 70;
  const int LT_X = CENTER_X + TRIG_PAD;
  const int LT_W = (CENTER_W / 2) - (TRIG_PAD + TRIG_PAD / 2);
  const int RT_X = CENTER_X + CENTER_W / 2 + TRIG_PAD / 2;
  const int RT_W = LT_W;
  const int TRIG_Y = PANEL_TOP + TRIG_PAD;

  // Button grid inside the center column
  const int BTN_GRID_TOP = PANEL_TOP + TRIG_H + TRIG_PAD * 2;
  const int BTN_SIZE = 36;
  const int BTN_GAP = 6;
  const int BTN_GRID_X = CENTER_X + (CENTER_W - (BTN_SIZE * 4 + BTN_GAP * 3)) / 2;

  // Deadzone strip (bottom bar)
  const int DZ_STRIP_TOP = PANEL_TOP + PANEL_H + 4;
  const int DZ_LABEL_W = 80;
  const int DZ_VAL_W = 60;
  const int DZ_SLIDER_X = DZ_LABEL_W + STRIP_PAD;
  const int DZ_SLIDER_W = WIN_W - DZ_LABEL_W - DZ_VAL_W - STRIP_PAD * 2;
}

ControllerVisualizerApp::ControllerVisualizerApp() 
  : ctrl_gamepad(0)
  , panel_left ({ 
                 Layout::LSTICK_X, 
                 Layout::PANEL_TOP, 
                 Layout::LSTICK_X + Layout::LSTICK_W,
                 Layout::PANEL_TOP + Layout::PANEL_H 
               }, L"LEFT STICK", L"L")
  , panel_right({
                 Layout::RSTICK_X,
                 Layout::PANEL_TOP,
                 Layout::RSTICK_X + Layout::RSTICK_W,
                 Layout::PANEL_TOP + Layout::PANEL_H
                }, L"RIGHT STICK", L"R")
  , panel_lt   ({
                 Layout::LT_X, 
                 Layout::TRIG_Y,
                 Layout::LT_X + Layout::LT_W,
                 Layout::TRIG_Y + Layout::TRIG_H
                }, L"L")
  , panel_rt   ({
                 Layout::RT_X,
                 Layout::TRIG_Y,
                 Layout::RT_X + Layout::RT_W,
                 Layout::TRIG_Y + Layout::TRIG_H
                }, L"R")
  , bWasConnected(FALSE)
  , hbr_background(NULL)
  , cmb_player(nullptr)
  , lbl_status(nullptr)
  , trk_deadzone(nullptr)
  , lbl_deadzone_val(nullptr)
  , lbl_playerTxt(nullptr)
  , lbl_deadzoneTxt(nullptr)
{}

// ────── ⋆⋅☆⋅⋆ ────────
// OnCreate
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::OnCreate() {
  const int iY = Layout::STRIP_PAD;

  // player slot label
  lbl_playerTxt = new Label(hwnd_self, L"Player:", Layout::STRIP_PAD, iY + 2, 52, 20);

  // Player slot ComboBox (lists slots 0-3)
  cmb_player = new ComboBox(hwnd_self, ID_CMB_PLAYER, Layout::STRIP_PAD + 56, iY, 60, 120);
  cmb_player->AddItem(L"0");
  cmb_player->AddItem(L"1");
  cmb_player->AddItem(L"2");
  cmb_player->AddItem(L"3");
  cmb_player->SelectFirst();

  // Status label
  lbl_status = new Label(hwnd_self, L"Status: Disconnected", Layout::STRIP_PAD + 130, iY + 2, 200, 20);

  // Deadzone strip
  const int iDzY = Layout::DZ_STRIP_TOP + Layout::STRIP_PAD;

  lbl_deadzoneTxt = new Label(hwnd_self, L"Deadzone:", Layout::STRIP_PAD, iDzY + 2, Layout::DZ_LABEL_W, 20);

  trk_deadzone = new Trackbar(hwnd_self, ID_TRK_DEADZONE, Layout::DZ_SLIDER_X, iDzY, Layout::DZ_SLIDER_W, 24, 0, 32767);

  trk_deadzone->SetPos(ctrl_gamepad.GetDeadzone());
  trk_deadzone->SetTickFreq(3276); // ~10 ticks across range

  lbl_deadzone_val = new Label(hwnd_self, L"", Layout::DZ_SLIDER_X + Layout::DZ_SLIDER_W + Layout::STRIP_PAD, iDzY + 2, Layout::DZ_VAL_W, 20);
  UpdateDeadzoneLabel();

  // Paint the window background dark. Store the brush so OnDestroy can free it.
  hbr_background = CreateSolidBrush(CLR_APP_BG);
  SetClassLongPtr(hwnd_self, GCLP_HBRBACKGROUND, (LONG_PTR)hbr_background);

  SetTimer(hwnd_self, ID_TIMER_POLL, 16, NULL);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnDestroy
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::OnDestroy() {

  KillTimer(hwnd_self, ID_TIMER_POLL);
  
  delete cmb_player;
  delete lbl_status;
  delete trk_deadzone;
  delete lbl_deadzone_val;
  delete lbl_playerTxt;
  delete lbl_deadzoneTxt;
  
  if(hbr_background) {
    DeleteObject(hbr_background);
    hbr_background = NULL;
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnPaint
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::OnPaint(HDC hdc) {
  panel_left.Draw(hdc, ctrl_gamepad.GetLX(), ctrl_gamepad.GetLY(), ctrl_gamepad.GetRawLX(), ctrl_gamepad.GetRawLY(), ctrl_gamepad.GetDeadzone());
  panel_right.Draw(hdc, ctrl_gamepad.GetRX(), ctrl_gamepad.GetRY(), ctrl_gamepad.GetRawRX(), ctrl_gamepad.GetRawRY(), ctrl_gamepad.GetDeadzone());

  panel_lt.Draw(hdc, ctrl_gamepad.GetLeftTrigger());
  panel_rt.Draw(hdc, ctrl_gamepad.GetRightTrigger());

  DrawButtonGrid(hdc);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnCommand
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::OnCommand(int iControlId, int iNotifCode) {
  if(iControlId == ID_CMB_PLAYER && iNotifCode == CBN_SELCHANGE) {
    wchar_t arrBuf[4];
    cmb_player->GetSelected(arrBuf, 4);
    int iSlot = (int)(arrBuf[0] - L'0');

    // Reconstruct the controller witht the new player index
    ctrl_gamepad = XInputController(iSlot);
    ctrl_gamepad.SetDeadzone(trk_deadzone->GetPos());
    UpdateStatus();
    InvalidateRect(hwnd_self, NULL, TRUE);
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnTimer
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::OnTimer(int iTimerId) {
  if(iTimerId != ID_TIMER_POLL) return;

  BOOL bConnected = ctrl_gamepad.Poll();

  // Update status label only on connection state transition
  if(bConnected != bWasConnected) {
    bWasConnected = bConnected;
    UpdateStatus();
  }

  // Alaways repaint so stick/trigger/button state stays live
  InvalidateRect(hwnd_self, NULL, FALSE);
}

// ────── ⋆⋅☆⋅⋆ ────────
// OnScroll
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::OnScroll(HWND hwnd_control, int iCode) {
  if(hwnd_control == trk_deadzone->GetHandle()) {
    ctrl_gamepad.SetDeadzone(trk_deadzone->GetPos());
    UpdateDeadzoneLabel();
    InvalidateRect(hwnd_self, NULL, FALSE);
  }
}

// ────── ⋆⋅☆⋅⋆ ────────
// DrawButtonGrid
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::DrawButtonGrid(HDC hdc) const {
  const int iX = Layout::BTN_GRID_X;
  const int iY = Layout::BTN_GRID_TOP;
  const int iSz = Layout::BTN_SIZE;
  const int iG = Layout::BTN_GAP;

  // Row 0: face buttons A B X Y
  DrawButton(hdc, iX, iY, iSz, iSz, L"A", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_A));
  DrawButton(hdc, iX + (iSz + iG), iY, iSz, iSz, L"B", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_B));
  DrawButton(hdc, iX + (iSz + iG) * 2, iY, iSz, iSz, L"X", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_X));
  DrawButton(hdc, iX + (iSz + iG) * 3, iY, iSz, iSz, L"Y", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_Y));

  // Row 1: bumpers + Start/Select
  const int iY1 = iY + iSz + iG;
  DrawButton(hdc, iX, iY1, iSz, iSz, L"LB", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_LEFT_SHOULDER));
  DrawButton(hdc, iX + (iSz + iG), iY1, iSz, iSz, L"RB", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER));
  DrawButton(hdc, iX + (iSz + iG) * 2, iY1, iSz, iSz, L"St", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_START));
  DrawButton(hdc, iX + (iSz + iG) * 3, iY1, iSz, iSz, L"Se", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_BACK));

  // Row 2: D-pad
  const int iY2 = iY1 + iSz + iG;
  DrawButton(hdc, iX, iY2, iSz, iSz, L"↑", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_DPAD_UP));
  DrawButton(hdc, iX + (iSz + iG), iY2, iSz, iSz, L"↓", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_DPAD_DOWN));
  DrawButton(hdc, iX + (iSz + iG) * 2, iY2, iSz, iSz, L"←", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_DPAD_LEFT));
  DrawButton(hdc, iX + (iSz + iG) * 3, iY2, iSz, iSz, L"→", ctrl_gamepad.IsPressed(XINPUT_GAMEPAD_DPAD_RIGHT));
}

// ────── ⋆⋅☆⋅⋆ ────────
// Draw Button
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::DrawButton(HDC hdc, int iX, int iY, int iW, int iH, const wchar_t* szLabel, BOOL bPressed) const {
  COLORREF clrFill = bPressed ? CLR_BTN_ACTIVE : CLR_BTN_INACTIVE;

  HBRUSH hBrush = CreateSolidBrush(clrFill);
  HPEN hPen = CreatePen(PS_SOLID, 1, CLR_BTN_BORDER);
  HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
  HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

  Rectangle(hdc, iX, iY, iX + iW, iY + iH);

  SelectObject(hdc, hOldBrush);
  SelectObject(hdc, hOldPen);
  DeleteObject(hBrush);
  DeleteObject(hPen);

  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, CLR_BTN_TEXT);

  RECT rc = {
    iX,
    iY,
    iX + iW,
    iY + iH
  };

  DrawText(hdc, szLabel, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// ────── ⋆⋅☆⋅⋆ ────────
// UpdateStatus
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::UpdateStatus() {
  lbl_status->SetText(ctrl_gamepad.IsConnected() ? L"Status: Connected" : L"Status: Disconnected");
}

// ────── ⋆⋅☆⋅⋆ ────────
// UpdateDeadzoneLabel
// ────── ⋆⋅☆⋅⋆ ────────
void ControllerVisualizerApp::UpdateDeadzoneLabel() {
  wchar_t arrBuf[16];
  wsprintf(arrBuf, L"%d", trk_deadzone->GetPos());
  lbl_deadzone_val->SetText(arrBuf);
}