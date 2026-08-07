/// @file DualMotorApp.cpp

#include "DualMotorApp.h"

namespace {
  const COLORREF CLR_APP_BG = RGB(30, 30, 30);
}

// Layout constants
namespace Layout {
  // Control strip
  const int STRIP_H = 36;
  const int STRIP_PAD = 8;

  // Motor Panels
  const int PANEL_TOP = STRIP_H + 4;
  const int PANEL_H = 380;
  const int PANEL_W = 580 / 2;

  // Deadzone strip
  const int DZ_STRIP_TOP = PANEL_TOP + PANEL_H + 4;
  const int DZ_LABEL_W = 80;
  const int DZ_VAL_W = 60;
  const int DZ_SLIDER_X = DZ_LABEL_W + STRIP_PAD;
  const int DZ_SLIDER_W = 600 - DZ_LABEL_W - DZ_VAL_W - STRIP_PAD * 2;
}

// PWM constants
namespace PWM {
  const int MIN_SPEED = 128;
  const int MAX_SPEED = 255;
}

DualMotorApp::DualMotorApp()
  : ctrl_gamepad(0)
  , panel_motor1({ 0, Layout::PANEL_TOP, Layout::PANEL_W, Layout::PANEL_TOP + Layout::PANEL_H }, L"MOTOR 1 (Left Stick)")
  , panel_motor2({ Layout::PANEL_W + 10, Layout::PANEL_TOP, 600, Layout::PANEL_TOP + Layout::PANEL_H  }, L"MOTOR 2 (Right Stick)")
  , cmb_port(nullptr)
  , btn_connect(nullptr)
  , btn_disc(nullptr)
  , lbl_status(nullptr)
  , trk_deadzone(nullptr)
  , lbl_deadzone_val(nullptr)
  , lbl_port_txt(nullptr)
  , lbl_deadzone_txt(nullptr)
  , iLastSpeed1(0)
  , iLastSpeed2(0)
  , hbr_background(NULL)
{}

DualMotorApp::~DualMotorApp() {
  delete cmb_port;
  delete btn_connect;
  delete btn_disc;
  delete lbl_status;
  delete trk_deadzone;
  delete lbl_deadzone_val;
  delete lbl_port_txt;
  delete lbl_deadzone_txt;
}

void DualMotorApp::OnCreate() {
  const int iY = Layout::STRIP_PAD;

  // Port label and combobox
  lbl_port_txt = new Label(hwnd_self, L"Port", Layout::STRIP_PAD, iY + 2, 36, 20);
  cmb_port = new ComboBox(hwnd_self, ID_CMB_PORT, Layout::STRIP_PAD + 40, iY, 90, 120);
  ScanComPorts(cmb_port);

  // Connect/Disconnect
  btn_connect = new Button(hwnd_self, L"Connect", ID_BTN_CONNECT, Layout::STRIP_PAD + 138, iY, 90, 22);
  btn_disc = new Button(hwnd_self, L"Disconnect", ID_BTN_DISCONNECT, Layout::STRIP_PAD + 224, iY, 90, 22);
  btn_disc->Disable();

  // Status Label
  lbl_status = new Label(hwnd_self, L"Status: Disconnected", Layout::STRIP_PAD + 322, iY + 2, 220, 20);

  // Deadzone strip
  const int iDzY = Layout::DZ_STRIP_TOP + Layout::STRIP_PAD;
  lbl_deadzone_txt = new Label(hwnd_self, L"Deadzone:", Layout::STRIP_PAD, iDzY + 2, Layout::DZ_LABEL_W, 20);
  trk_deadzone = new Trackbar(hwnd_self, ID_TRK_DEADZONE, Layout::DZ_SLIDER_X, iDzY, Layout::DZ_SLIDER_W, 24, 0, 32787);
  trk_deadzone->SetPos(ctrl_gamepad.GetDeadzone());
  trk_deadzone->SetTickFreq(3276);
  lbl_deadzone_val = new Label(hwnd_self, L"", Layout::DZ_SLIDER_X + Layout::DZ_SLIDER_W + Layout::STRIP_PAD, iDzY + 2, Layout::DZ_VAL_W, 20);
  UpdateDeadzoneLabel();

  // Dark background
  hbr_background = CreateSolidBrush(CLR_APP_BG);
  SetClassLongPtr(hwnd_self, GCLP_HBRBACKGROUND, (LONG_PTR)hbr_background);

  SetTimer(hwnd_self, ID_TIMER_POLL, 18, NULL);
}

void DualMotorApp::OnDestroy() {
  KillTimer(hwnd_self, ID_TIMER_POLL);

  if(serial_port.IsOpen()) {
    SendMotorCommand(0, 0);
    serial_port.Close();
  }

  if(hbr_background) {
    DeleteObject(hbr_background);
    hbr_background = NULL;
  }
}

void DualMotorApp::OnPaint(HDC hdc) {
  panel_motor1.Draw(hdc, iLastSpeed1);
  panel_motor2.Draw(hdc, iLastSpeed2);
}

void DualMotorApp::OnCommand(int iControlId, int iNotifCode) {
  if(iControlId == ID_BTN_CONNECT) {
    wchar_t arrPort[32];
    cmb_port->GetSelected(arrPort, 32);

    if(serial_port.Open(arrPort, 9600)) {
      btn_connect->Disable();
      btn_disc->Enable();
      UpdateStatus();
    } else {
      MessageBox(hwnd_self, L"Failed to open port. Check Connection and try again.", L"Connection Error", MB_OK | MB_ICONERROR);
    }
  }

  if(iControlId == ID_BTN_DISCONNECT) {
    SendMotorCommand(0, 0);
    serial_port.Close();
    btn_connect->Enable();
    btn_disc->Disable();
    iLastSpeed1 = 0;
    iLastSpeed2 = 0;
    UpdateStatus();
    InvalidateRect(hwnd_self, NULL, FALSE);
  }
}

void DualMotorApp::OnTimer(int iTimerId) {
  if(iTimerId != ID_TIMER_POLL) {
    return;
  }

  ctrl_gamepad.Poll();

  // Map both stick Y axes to PWM speeds
  int iSpeed1 = StickToPWM(ctrl_gamepad.GetLY());
  int iSpeed2 = StickToPWM(ctrl_gamepad.GetRY());

  // Only send serial command if speed changed
  if(iSpeed1 != iLastSpeed1 || iSpeed2 != iLastSpeed2) {
    iLastSpeed1 = iSpeed1;
    iLastSpeed2 = iSpeed2;

    if(serial_port.IsOpen()) {
      SendMotorCommand(iSpeed1, iSpeed2);
    }

    InvalidateRect(hwnd_self, NULL, FALSE);
  }
}

void DualMotorApp::OnScroll(HWND hwnd_control, int iCode) {
  if(hwnd_control == trk_deadzone->GetHandle()) {
    ctrl_gamepad.SetDeadzone(trk_deadzone->GetPos());
    UpdateDeadzoneLabel();
  }
}

int DualMotorApp::StickToPWM(SHORT sStickY) {
  if(sStickY == 0) {
    return 0;
  }

  // XInputController already zeroes values inside the deadzone
  // non-zero values here are outside the deadzone
  // Remap[1,32787] -> [PWM_MIN, PWM_MAX]
  int iAbs = sStickY < 0 ? -sStickY : sStickY;
  int iSign = sStickY > 0 ? 1 : -1;
  float fNorm = (float)iAbs / 32787.0f;
  int iPWM = PWM::MIN_SPEED + (int)(fNorm * (PWM::MAX_SPEED - PWM::MIN_SPEED));

  // Clamp to valid range
  if(iPWM > PWM::MAX_SPEED) {
    iPWM = PWM::MAX_SPEED;
  }

  return iSign * iPWM;
}

void DualMotorApp::SendMotorCommand(int iSpeed1, int iSpeed2) {
  char arrBuf[32];
  wsprintfA(arrBuf, "M1:%d,M2:%d", iSpeed1, iSpeed2);
  serial_port.Write(arrBuf);
}

void DualMotorApp::UpdateStatus() {
  lbl_status->SetText(serial_port.IsOpen() ? L"Status: Connected" : L"Status: Disconnected");
}

void DualMotorApp::UpdateDeadzoneLabel() {
  wchar_t arrBuf[16];
  wsprintf(arrBuf, L"%d", trk_deadzone->GetPos());
  lbl_deadzone_val->SetText(arrBuf);
}