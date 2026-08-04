/// @file ACMotorApp.h
/// @brief Main window for the AC induction motor visualizer.
///   Owns the input controls, phase sequence toggle, WM_TIMER,
///   driven field angle accumulator, and readout strip.

#pragma once
#include "MotorMath.h"
#include "TorqueSpeedPanel.h"
#include "RotatingFieldPanel.h"
#include "..\..\..\pch.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_INPUT_V 101
#define ID_INPUT_FREQ 102
#define ID_INPUT_POLES 103
#define ID_INPUT_R2 104
#define ID_INPUT_X2 105
#define ID_BTN_SEQ 106
#define ID_TIMER_ANIM 1

// Timer interval in milliseconds (~60fps)
static const int TIMER_MS = 16;

class ACMotorApp : public Window {
  public:
    ACMotorApp();
    ~ACMotorApp();

  protected:
    void OnCreate() override;
    void OnDestroy() override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnPaint(HDC hdc) override;
    void OnTimer(int iTimerId) override;
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

  private:
    // Layout
    void LayoutControls(int iW, int iH);
    RECT GetTorquePanelRect() const;
    RECT GetFieldPanelRect() const;

    // Recalculation
    void RecalcAndRedraw();
    void UpdateReadouts();
    float ReadInputFloat(TextInput* pInput, float fDefault = 0.0f) const;
    int ReadInputInt(TextInput* pInput, int iDefault = 0) const;

    // Input controls
    Label* lbl_V;
    TextInput* txti_V;
    Label* lbl_Freq; 
    TextInput* txti_Freq;
    Label* lbl_Poles;
    TextInput* txti_Poles;
    Label* lbl_R2;
    TextInput* txti_R2;
    Label* lbl_X2;
    TextInput* txti_X2;

    // Toggle button
    Button* btn_seq; // ABC -> ACB

    // Readout strip
    Label* lbl_out_Ns;
    Label* lbl_out_Nr;
    Label* lbl_out_slip;
    Label* lbl_out_Tbreak;
    Label* lbl_out_Tstart;
    Label* lbl_out_sbreak;
    Label* lbl_status;

    // State
    MotorInputs m_inputs;
    MotorState m_state;

    // Animation
    float m_fTheta_rad; // advances fOmega_s * dt on every timer tick
    float m_fAnimDir; // +1.0 for ABC, -1.0 for ACB

    // GDI panels
    TorqueSpeedPanel m_torquePanel;
    RotatingFieldPanel m_fieldPanel;

    // Layout constants
    static const int CONTROLS_H = 68;
    static const int READOUT_H = 38;
    static const int MARGIN = 8;
};