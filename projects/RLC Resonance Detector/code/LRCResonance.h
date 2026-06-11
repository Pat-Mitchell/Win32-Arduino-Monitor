/// @file LRCResonance.h
/// @brief LRC Resonance Detector
///   Sends sweep commands to Arduino, plots frequency response,
///   detects resonant peak, and compares against theoretical f_0.

#include "FreqPlotPanel.h"
#include "../../../pch.h"
#include "../../../Utils/Utils.h"
#include "../../../wrappers/win32Wrappers/Window.h"
#include "../../../wrappers/win32Wrappers/Button.h"
#include "../../../wrappers/win32Wrappers/Label.h"
#include "../../../wrappers/win32Wrappers/TextInput.h"
#include "../../../wrappers/win32Wrappers/ComboBox.h"
#include "../../../wrappers/win32Wrappers/ProgressBar.h"
#include "../../../wrappers/serialPortWrappers/SerialPort.h"

#include <CommCtrl.h>
#include <math.h>
#pragma comment(lib, "comct132.lib")

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────

#define ID_COMBO_PORT     101
#define ID_BTN_CONNECT    102
#define ID_BTN_DISCONNECT 103
#define ID_EDIT_FSTART    104
#define ID_EDIT_FEND      105
#define ID_EDIT_STEPS     106
#define ID_BTN_SWEEP      107
#define ID_BTN_STOP       108
#define ID_EDIT_LVAL      109
#define ID_EDIT_CVAL      110
#define ID_EDIT_VPIN      111
#define ID_BTN_SETVPIN    112
#define ID_PROGRESS       113
#define ID_BTN_EXPORT     114
#define ID_TIMER_POLL     1

#define POLL_MS     50
#define READ_BUF    512
#define MAX_SAMPLES 512

// ────── ⋆⋅☆⋅⋆ ────────
// LRCWindow
// ────── ⋆⋅☆⋅⋆ ────────

class LRCWindow : public Window {
  public:
    LRCWindow();
    ~LRCWindow();

  protected:
    void OnCreate() override;
    void OnPaint(HDC hdc) override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnTimer(int iTimerId) override;
    void OnDestroy() override;

  private:
    bool ParseLine(const wchar_t* szLine);
    void StartSweep();
    void UpdateTheoreticalF0();
    void UpdateError(float fF0_measured);
    void OnConnect();
    void OnDisconnect();

    // ────── ⋆⋅☆⋅⋆ ────────
    // Members
    // ────── ⋆⋅☆⋅⋆ ────────
    SerialPort port;
    FreqPlotPanel plot;

    char arrLineBuf[READ_BUF * 4];
    int iLineBufLen;

    bool bSweepActive;
    int iExpectedSteps;
    int iReceivedSteps;
    float fVpin;

    ComboBox* cmb_port;
    Button* btn_connect;
    Button* btn_disc;
    TextInput* edit_fstart;
    TextInput* edit_fend;
    TextInput* edit_steps;
    Button* btn_sweep;
    Button* btn_stop;
    TextInput* edit_lval;
    TextInput* edit_cval;
    TextInput* edit_vpin;
    Button* btn_setvpin;
    ProgressBar* prog_sweep;
    Button* btn_export;
    Label* lbl_f0_meas;
    Label* lbl_f0_theory;
    Label* lbl_error;
    Label* lbl_status;
    Label* lbl_vcc;
};