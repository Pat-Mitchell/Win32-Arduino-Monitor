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
#define IF_BTN_TRIG_FALL 106
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
    void init(int iX, int iY, int iW, int iH);
    void clear();

    /// @brief Loads a captured frame and computes V_pp, V_rms, frequency
    void loadFrame(const int* arrData, int iN, float fVcc, float fGain, float fVbias, int iTrig, bool bTriggered);

    /// @brief Updates display-only parmeters (gain/bias) without a new frame.
    ///   Called when the user changes settings and clicks apply.
    void updateDisplayParams(float fNewGain, float fNewVbias);

    void draw(HDC hdc) const;

    /// @brief Converts an ADC const to the recovered input signal voltage
    ///   V_signal = (V_bias - V_out) / Gain
    ///   Returns - when ADC reads V_bias (no signal), positive when the
    ///   inverting output is below bias (input going positive).
    float adcToVsig(int iAdc) const;
  
  private:
    void computeMeasurements();
    int mapX(int iIdx) const;
    int mapY(float fVsig, float fVmin, float fVmax) const;
};