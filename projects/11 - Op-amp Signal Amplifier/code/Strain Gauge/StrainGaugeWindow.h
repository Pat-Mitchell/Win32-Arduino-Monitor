/// @file StrainGaugeWindow.h

#pragma once

#include "../../../../pch.h"
#include "../../../../Utils/Utils.h"
#include "../../../../wrappers/win32Wrappers/Window.h"
#include "../../../../wrappers/win32Wrappers/Button.h"
#include "../../../../wrappers/win32Wrappers/Label.h"
#include "../../../../wrappers/win32Wrappers/TextInput.h"
#include "../../../../wrappers/win32Wrappers/ComboBox.h"
#include "../../../../wrappers/serialPortWrappers/SerialPort.h"

#include "StrainPlotPanel.h"

#define ID_COMBO_PORT       101
#define ID_BTN_CONNECT      102
#define ID_BTN_DISCONNECT   103
#define ID_BTN_TARE         104
#define ID_BTN_RESET        105
#define ID_BTN_APPLY        106
#define ID_BTN_EXPORT       107
#define ID_TIMER_POLL       1

#define POLL_MS     100
#define READ_BUF    512
#define MAX_SAMPLES 512

class StrainGaugeWindow : public Window {
  public:
    StrainGaugeWindow();
    ~StrainGaugeWindow();

  protected:
    void OnCreate() override;
    void OnPaint(HDC hdc) override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnTimer(int iTimerId) override;
    void OnDestroy() override;

  private:
    bool ParseLine(const wchar_t* szLine);
    void UpdateReadouts(int iDelta, float fVadc);
    void ApplySettings();
    void OnConnect();
    void OnDisconnect();

    // Sensor parameters (user-entered)
    float fGain; // Op-amp gain (Rf / Rin)
    float fGF; // Gauge factor (typically 2.0)
    float fE_GPa; // Young's modulus of Bar material (Al = 69 GPa)
    float fH_mm; // Bar thickness in mm
    float fL_mm; // Bar gauge length (distance from clamp to gauge) mm

    // Runtime state
    int iADC_tare;
    bool bTared;
    float fVcc;
    long lRecStart;

    SerialPort port;
    StrainPlotPanel plot;

    char arrLineBuf[READ_BUF * 4];
    int iLineBufLen;

    // Controls
    ComboBox* cmb_port;
    Button* btn_connect;
    Button* btn_disc;
    Button* btn_tare;
    Button* btn_reset;
    TextInput* edit_gain;
    TextInput* edit_gf;
    TextInput* edit_elastic;
    TextInput* edit_h;
    TextInput* edit_l;
    Button* btn_apply;
    Button* btn_export;

    // Readout labels
    Label* lbl_adc_val;
    Label* lbl_vadc_val;
    Label* lbl_delta_val;
    Label* lbl_strain_val;
    Label* lbl_stress_val;
    Label* lbl_defl_val;
    Label* lbl_tare_status;
    Label* lbl_vcc_val;
};