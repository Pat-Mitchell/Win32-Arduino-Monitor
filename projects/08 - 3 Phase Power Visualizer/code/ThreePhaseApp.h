/// @file ThreePhaseApp.h
/// @brief Main window for the 3-Phase Power Visualizer

#pragma once
#include "ThreePhaseMath.h"
#include "WaveformPanel.h"
#include "PhasorPanel3.h"
#include "..\..\..\pch.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_INPUT_V 101
#define ID_INPUT_FREQ 102
#define ID_INPUT_R 103
#define ID_BTN_CONFIG 104
#define ID_BTN_SEQ 105

class ThreePhaseApp : public Window {
  public:
    ThreePhaseApp();
    ~ThreePhaseApp();

  protected:
    void OnCreate() override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnPaint(HDC hdc) override;
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

  private:
    // Layout
    void LayoutControls(int iW, int iH);
    RECT GetWaveformPanelRect() const;
    RECT GetPhasorPanelRect() const;

    // Recalculation
    void RecalcAndRedraw();
    void UpdateReadouts();
    float ReadInputFloat(TextInput* pInput, float fDefault = 0.0f) const;

    // Input controls
    Label* lbl_V;
    TextInput* txti_V;
    Label* lbl_Freq;
    TextInput* txti_Freq;
    Label* lbl_R;
    TextInput* txti_R;

    // Toggle buttons
    Button* btn_config; // Star and Delta
    Button* btn_seq; // ABC and ACB

    // Readout strip
    Label* lbl_out_Vph;
    Label* lbl_out_VL;
    Label* lbl_out_Iph;
    Label* lbl_out_IL;
    Label* lbl_out_P;
    Label* lbl_out_neutral;
    Label* lbl_out_config;
    Label* lbl_status;

    // State
    ThreePhaseInputs m_inputs;
    ThreePhaseState m_state;

    // GDI panels
    WaveformPanel m_waveformPanel;
    PhasorPanel3 m_phasorPanel;

    // Layout constants
    static const int CONTROLS_H = 64;
    static const int READOUT_H = 40;
    static const int MARGIN = 8;
};