/// @file ACPhasor.h
/// @brief Main window for the AC Phasor Diagram application.
///   Owns the input controls, readout strip, and coordinates
///   recalculation and repainting on every input change.

#pragma once
#include "phasorMath.h"
#include "phasorPanel.h"
#include "PowerTrianglePanel.h"
#include "../../pch.h"
#include "../../Utils/Utils.h"
#include "../../wrappers/win32Wrappers/Window.h"
#include "../../wrappers/win32Wrappers/Button.h"
#include "../../wrappers/win32Wrappers/Label.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_INPUT_R 101
#define ID_INPUT_L 102
#define ID_INPUT_C 103
#define ID_INPUT_FREQ 104
#define ID_INPUT_V 105

class ACPhasorApp : public Window {
  public:
    ACPhasorApp();
    ~ACPhasorApp();

  protected:
    void OnCreate() override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnPaint(HDC hdc) override;
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

  private:
    // Layout
    void LayoutControls(int iClientW, int iClientH);
    RECT GetPhasorPanelRect() const;
    RECT GetPowerPanelRect() const;

    // Recalculation
    void RecalcAndRedraw();
    void UpdateReadouts();
    float ReadInputFloat(TextInput* pInput, float fDefault = 0.0f) const;

    // Input controls
    Label* lbl_R; TextInput* txti_R;
    Label* lbl_L; TextInput* txti_L;
    Label* lbl_C; TextInput* txti_C;
    Label* lbl_Freq; TextInput* txti_Freq;
    Label* lbl_V; TextInput* txti_V;

    // Readouts
    Label* lbl_out_Z;
    Label* lbl_out_Phi;
    Label* lbl_out_PF;
    Label* lbl_out_I;
    Label* lbl_out_P;
    Label* lbl_out_Q;
    Label* lbl_out_S;
    Label* lbl_status;

    // State
    PhasorInputs m_inputs;
    PhasorState m_state;

    // GDI panels
    PhasorPanel m_phasorPanel;
    PowerTrianglePanel m_powerPanel;

    // Layout constants
    static const int INPUT_AREA_H = 72;
    static const int READOUT_AREA_H = 56;
    static const int MARGIN = 8;
};