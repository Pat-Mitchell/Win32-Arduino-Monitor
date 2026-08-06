/// @file DualMotorApp.h
/// @brief Amin application window for the Dual Motor Controller App.
///   Reads both thumbstick Y-axes via XInput and sends independent
///   speed/direction commands to two DC motors over serial
///
///   Left stick -> Motor 1
///   Right Stick -> Motor 2
///
///   PWM mapping:
///      Deazone: 0%
///      Outside Deadzone: [50%, 100%]
///      Scale linearly from 50% to 100%

#pragma once
#include "../../../pch.h"
#include "../../../Utils/Utils.h"
#include "../../../wrappers/xinputWrappers/XInputController.h"
#include "../../../wrappers/win32Wrappers/Window.h"
#include "../../../wrappers/win32Wrappers/Button.h"
#include "../../../wrappers/win32Wrappers/Label.h"
#include "../../../wrappers/win32Wrappers/TextInput.h"
#include "../../../wrappers/win32Wrappers/ComboBox.h"
#include "../../../wrappers/win32Wrappers/Trackbar.h"
#include "../../../wrappers/serialPortWrappers/SerialPort.h"
#include "MotorPanel.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_CMB_PORT 101
#define ID_BTN_CONNECT 102
#define ID_BTN_DISCONNECT 103
#define ID_TRK_DEADZONE 104
#define ID_TIMER_POLL 1

class DualMotorApp : public Window {
  public:
    DualMotorApp();
    ~DualMotorApp();

  protected:
    void OnCreate() override;
    void OnDestroy() override;
    void OnPaint(HDC hdc) override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnTimer(int iTimerId) override;
    void OnScroll(HWND hwnd_control, int iCode) override;

  private:
    /// @brief Maps a filtered stick Y value to a PWM value
    /// @return PWM speed in [-255, +255] (inside deadzone = 0)
    static int StickToPWM(SHORT sStickY);


    // Formats and sends a motor speed from over serial
    void SendMotorCommand(int iSpeed1, int iSpeed2);

    // Updates the status label to reflect serial connection state
    void UpdateStatus();

    // Updates the numeric deadzone label besides the slider
    void UpdateDeadzoneLabel();

    // Player 0 controller
    XInputController ctrl_gamepad;

    // GDI panels
    MotorPanel panel_motor1;
    MotorPanel panel_motor2;

    // ────── ⋆⋅☆⋅⋆ ────────
    // Win32 controls
    // ────── ⋆⋅☆⋅⋆ ────────
    ComboBox* cmb_port;
    Button* btn_connect;
    Button* btn_disc;
    Label* lbl_status;
    Trackbar* trk_deadzone;
    Label* lbl_deadzone_val;
    Label* lbl_port_txt;
    Label* lbl_deadzone_txt;

    // Serial port
    SerialPort serial_port;

    // State
    // Previous speed avoid redundant serial writes
    int iLastSpeed1;
    int iLastSpeed2;
    HBRUSH hbr_background;
};