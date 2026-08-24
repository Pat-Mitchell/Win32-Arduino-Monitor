/// @file Oscilloscope.h
/// @brief Op-amp signl amplifier Oscilloscope.
///   Trigged waveform display with auto-scaling Y-axis,
///   trigger level control, and V_pp / V_rms / frequency readouts

#pragma once

#include "../../../../pch.h"
#include "../../../../Utils/Utils.h"
#include "../../../../wrappers/win32Wrappers/Window.h"
#include "../../../../wrappers/win32Wrappers/Button.h"
#include "../../../../wrappers/win32Wrappers/Label.h"
#include "../../../../wrappers/win32Wrappers/TextInput.h"
#include "../../../../wrappers/win32Wrappers/ComboBox.h"
#include "../../../../wrappers/win32Wrappers/Trackbar.h"
#include "../../../../wrappers/serialPortWrappers/SerialPort.h"

#include <CommCtrl.h>
#include <math.h>
#pragma comment(lib, "comctl32.lib")

// ────── ⋆⋅☆⋅⋆ ────────
// Constants
// ────── ⋆⋅☆⋅⋆ ────────
#define MAX_SCOPE_SAMPLES 512
#define SCOPE_SAMPLE_RATE 76000.0f // ~76kHz at ADC prescaler /16
#define SCOPE_BAUD_RATE 115200 // as defined by oscilloscope.ino

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_COMBO_PORT 101
#define ID_BTN_CONNECT 102
#define ID_BTN_DISCONNECT 103
#define ID_BTN_TRIG_TOGGLE 104
#define ID_BTN_TRIG_RISE 105
#define ID_BTN_TRIG_FALL 106
#define ID_TRACKBAR_TRIG 107
#define ID_EDIT_SAMPLES 108
#define ID_EDIT_GAIN 109
#define ID_EDIT_VBIAS 110
#define ID_BTN_APPLY 111
#define ID_BTN_EXPORT 112
#define ID_TIMER_POLL 1

#define POLL_MS 50
#define READ_BUF 1024

// ────── ⋆⋅☆⋅⋆ ────────
// Oscilloscope Panel
// ────── ⋆⋅☆⋅⋆ ────────
/// @brief GDI waveform display. Stores one frame of ADC samples and renders
///   a triggered oscilloscope view with grid, trigger line, and auto-scaling 
///   Y axis. All voltage calculations use the inverting amplifier model:
///   V_signal = (V_bias - V_out) / Gain.

struct OscilloscopePanel {
  public: 
    // Dataset
    int arrSamples[MAX_SCOPE_SAMPLES];
    int iCount;
    bool bHasFrame;

    // Frame metadata (from most recent FRAME: header)
    float fVcc;
    float fGain;
    float fVbias;
    int iTrigADC;
    bool bLastTriggered;

    // Computed measurements (updated in LoadFrame)
    float fLastVpp;
    float fLastVrms;
    float fLastFreq;

    // Layout
    RECT rect_bounds;
    int iPadL, iPadT, iPadR, iPadB;

    // ────── ⋆⋅☆⋅⋆ ────────
    // Public interface
    // ────── ⋆⋅☆⋅⋆ ────────
    void Init(int iX, int iY, int iW, int iH);
    void Clear();

    /// @brief Loads a captured frame and computes V_pp, V_rms, frequency
    void LoadFrame(const int* arrData, int iN, float fVcc, float fGain, float fVbias, int iTrig, bool bTriggered);

    /// @brief Updates display-only parmeters (gain/bias) without a new frame.
    ///   Called when the user changes settings and clicks apply.
    void UpdateDisplayParams(float fNewGain, float fNewVbias);

    void Draw(HDC hdc) const;

    /// @brief Converts an ADC const to the recovered input signal voltage
    ///   V_signal = (V_bias - V_out) / Gain
    ///   Returns - when ADC reads V_bias (no signal), positive when the
    ///   inverting output is below bias (input going positive).
    float AdcToVsig(int iAdc) const;
  
  private:
    void ComputeMeasurements();
    int MapX(int iIdx) const;
    int MapY(float fVsig, float fVmin, float fVmax) const;
};

// ────── ⋆⋅☆⋅⋆ ────────
// OscilloscopeWindow
// ────── ⋆⋅☆⋅⋆ ────────
class OscilloscopeWindow : public Window {
  public:
    OscilloscopeWindow();
    ~OscilloscopeWindow();

  protected:
    void OnCreate() override;
    void OnPaint(HDC hdc) override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnScroll(HWND hwnd_control, int iCode) override;
    void OnTimer(int iTimerId) override;
    void OnDestroy() override;

  private:
    // Parsing
    bool ParseLine(const wchar_t* szLine);
    bool ParseDataLine(const wchar_t* szLine, int* arrOut, int iExpected, int& iActual);

    // Ui helpers
    void UpdateTrigLabel(int iADC);
    void UpdateReadouts();
    void ApplySettings();
    void SetTriggerMode(bool bAuto);
    void OnConnect();
    void OnDisconnect();

    // Members
    SerialPort port;
    OscilloscopePanel panel;

    char arrLineBuf[READ_BUF * 4];
    int iLineBufLen;

    // Two-phase frame parsing
    bool bExpectingData;
    int iExpectedSamples;
    float fFrameVcc;
    float fFrameGain;
    int iFrameTrig;
    bool bFrameTriggered;

    // Current display settings
    float fCurrentVcc;
    float fCurrentGain;
    float fCurrentVbias;
    bool bAutoTrigger;

    // Controls
    ComboBox* cmb_port;
    Button* btn_connect;
    Button* btn_disc;
    Button* btn_trig_toggle;
    Button* btn_trig_rise;
    Button* btn_trig_fall;
    Trackbar* trk_trig;
    Label* lbl_trig_val;
    TextInput* edit_samples;
    TextInput* edit_gain;
    TextInput* edit_vbias;
    Button* btn_apply;
    Button* btn_export;

    Label* lbl_vpp_val;
    Label* lbl_vrms_val;
    Label* lbl_freq_val;
    Label* lbl_vcc_val;
    Label* lbl_status;
};