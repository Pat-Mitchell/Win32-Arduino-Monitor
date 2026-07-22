/// @file ControllerVisualizerApp.h
/// @brief Main application window for the Controller Input Visualizer.
///   Polls an XInput gamepad at ~60Hz and renders live stick, trigger, and button state using GDI panels.

#pragma once
#include "..\..\pch.h"
#include "..\..\wrappers\xinputWrappers\XInputController.h"
#include "StickPanel.h"
#include "TriggerPanel.h"

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDS
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_CMB_PLAYER 101
#define ID_TRK_DEADZONE 102
#define ID_TIMER_POLL 1

class ControllerVisualizerApp : public Window {
  public:
    ControllerVisualizerApp();
  
  protected:
    void OnCreate() override;
    void OnDestroy() override;
    void OnPaint(HDC hdc) override;
    void OnCommand(int iControlId, int iNotifCode) override;
    void OnTimer(int iTimerId) override;
    void OnScroll(HWND hwnd_control, int iCode) override;

  private:
    /// @brief Draws face buttons, bumpers, start/select, and D-pad as small colored
    ///   squares. Active buttons are highlighted. Inactive are dim.
    /// @param hdc Device context to draw into
    void DrawButtonGrid(HDC hdc) const;

    /// @brief Draws a single labelled button indicator square
    /// @param hdc Device context
    /// @param iX Left edge of the button square
    /// @param iY Top edge of the button square
    /// @param iW Width in pixels
    /// @param iH Height in pixels
    /// @param szLabel Short label drawn centered inside the square
    /// @param bPressed TRUE todraw the active state
    void DrawButton(HDC hdc, int iX, int iY, int iW, int iH, const wchar_t* szLabel, BOOL bPressed) const;

    /// @brief Updates the status label text to reflect connection state
    void UpdateStatus();

    /// @brief Updates the numeric deadzone label beside the slider
    void UpdateDeadzoneLabel();

    // ────── ⋆⋅☆⋅⋆ ────────
    // Members
    // ────── ⋆⋅☆⋅⋆ ────────

    // XInput wrapper for the active player slot
    XInputController ctrl_gamepad;

    // GDI panels
    StickPanel panel_left;
    StickPanel panel_right;
    TriggerPanel panel_lt;
    TriggerPanel panel_rt;

    // Win32 controls
    ComboBox* cmb_player;
    Label* lbl_status;
    Trackbar* trk_deadzone;
    Label* lbl_deadzone_val;
    Label* lbl_playerTxt;
    Label* lbl_deadzoneTxt;

    // State
    BOOL bWasConnected; // Connection state from precious tick
    HBRUSH hbr_background; // Dark background brush
};