/// @file XAudio2Syth.h
/// @brief XAudio2 wave generator
///   Generates sine, square, or sawtooth waveforms in a PCM buffer
///   and streams them to the default audio device via XAudio2
///   PlotPanel shows one period of the current waveform.

#pragma once

#include "../../pch.h"
#include "../../Utils/Utils.h"
#include "../../wrappers/win32Wrappers/D2DWindow.h"
#include "../../wrappers/win32Wrappers/Button.h"
#include "../../wrappers/win32Wrappers/Label.h"
#include "../../wrappers/win32Wrappers/TextInput.h"
#include "../../wrappers/win32Wrappers/D2DPlotPanel.h"

#include <xaudio2.h>
#include <cstdint>
#include <wchar.h>
#include <math.h>

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_EDIT_FREQ 101
#define ID_BTN_APPLY 102
#define ID_BTN_STARTSTOP 103
#define ID_RADIO_SINE 104
#define ID_RADIO_SQUARE 105
#define ID_RADIO_SAW 106

// ────── ⋆⋅☆⋅⋆ ────────
// Audio constants
// ────── ⋆⋅☆⋅⋆ ────────
#define SAMPLE_RATE 44100
#define AMPLITUDE 0x7FFF // 16-bit full scale = 32767
#define PLOT_POINTS 512

// ────── ⋆⋅☆⋅⋆ ────────
// Wave shape enum
// ────── ⋆⋅☆⋅⋆ ────────
enum WaveShape { SINE, SQUARE, SAWTOOTH };

// ────── ⋆⋅☆⋅⋆ ────────
// Wave Generation
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Fills arrBuf with iSamples of PCM audio for the given shape and freq.
///   16-bit signed mono PCM. Matches the WAVEFORMATEX set up
/// @param arrBuf Destination buffer (int16_t, iSamples long).
/// @param iSamples Number of samples to generate
/// @param fFreq_Hz Oscillator frequency in Hz
/// @param eShape Waveform shape
void GenerateWave(int16_t* arrBuf, int iSamples, float fFreq_Hz, WaveShape eShape);

/// @brief Fills float arrays for D2DPlotPanel showing exactly one period of
///   the waveform. X axis is time in milliseconds, Y axis is -1 to +1.
///   Uses float samples (no AMPLITUDE scaling) so Y range is clean.
/// @param arrX Output X values (time in ms for one period)
/// @param arrY Output Y values (-1.0 to +1.0)
/// @param Points Number of output points (PLOT_POINTS)
/// @param fFreq_Hz Frequency. Determines period length
/// @param eShape Wave shape
void BuildPlotData(float* arrX, float* arrY, int Points, float fFreq_Hz, WaveShape eShape);

// ────── ⋆⋅☆⋅⋆ ────────
// XAudio2Window
// ────── ⋆⋅☆⋅⋆ ────────
class XAudio2Window : public D2DWindow {
  public:
    XAudio2Window();
    ~XAudio2Window();

  protected:
    void OnCreate() override;
    void OnD2DPaint(ID2D1HwndRenderTarget* pRT) override;
    void OnSize(UINT iW, UINT iH) override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnDestroy() override;

  private:
    // Audio management
    void StartAudio();
    void StopAudio();
    void ReleaseXAudio2();
    void ApplyFrequency();
    void OnSettingsChanged();
    void RefreshPlot();
    void UpdateStatusLabel();

    // Members 
    
    // XAudio2 objects
    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasterVoice;
    IXAudio2SourceVoice* pSourceVoice;

    // 1 second of 16-bit mono PCM (the audio data)
    int16_t arrAudioBuf[SAMPLE_RATE];

    // Plot data (one waveform period in float)
    D2DPlotPanel* pPlot;
    float arrPlotX[PLOT_POINTS];
    float arrPlotY[PLOT_POINTS];

    float fFreq;
    WaveShape eShape;
    bool bPlaying;

    // Controls
    TextInput* edit_freq;
    Button* btn_apply;
    Button* btn_startstop;
    Label* lbl_status;

    // Radio buttons as raw HWNDs. Button wrapper uses BS_PUSHBUTTON
    // which is incompatible with BS_AUTORADIOBUTTON
    HWND hwnd_radio_sine;
    HWND hwnd_radio_square;
    HWND hwnd_radio_saw;
};