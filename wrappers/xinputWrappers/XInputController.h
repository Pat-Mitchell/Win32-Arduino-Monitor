/// @file XInputController.h
/// @brief Wraps the Win32 XInput API for a single gamepad.
///   Owns polling, connection state, button queries, and 
///   deadzone stick values. Designed to be held as a member
///   of a Window subclass, polled on WM_TIMER
///
/// @note Reuires xinput.lib. -lxinput needed in build command
/// @note Must use an Xbox controller or set controller to XInput mode

#pragma once
#include <windows.h>
#include <xinput.h>

class XInputController {
  public:
    /// @param iPlayerIndex XInput player index 0-3. 0 = Player 1
    explicit XInputController(int iPlayerIndex = 0);

    /// @brief Calls XInputGetState and updates internal state.
    ///    Call once per WM_TIMER tick before reading any values
    /// @return TRUE if the controller is connected, FALSE if disconnected
    BOOL Poll();
    BOOL IsConnected() const;

    /// @brief Check whether a button is currently held
    /// @param wButton One of the XINPUT_GAMEPAD_* bitmask constans
    ///   e.g. XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_LEFT_SHOULDER, etc.
    /// @return TRUE if the button is currently held
    BOOL IsPressed(WORD wButton) const;

    /// @brief Returns the raw left/right trigger value (0-255)
    BYTE GetLeftTrigger() const;
    BYTE GetRightTrigger() const;

    /// @brief Returns the raw SHORT value direct from xInput.
    ///   Includes hardware noise within the deadzone
    SHORT GetRawLX() const;
    SHORT GetRawLY() const;
    SHORT GetRawRX() const;
    SHORT GetRawRY() const;

    /// @brief Returns the deadzone-filtered stick value
    ///   Values inside the deadzone radius return 0
    ///   Values outside are rescaled so 0 starts at the deadzone edge
    /// @return return Filtered value in [-32767, +32767]
    SHORT GetLX() const;
    SHORT GetLY() const;
    SHORT GetRX() const;
    SHORT GetRY() const;
    
    /// @brief Sets the deadzone radius applied to both sticks
    ///   XInput recomends XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE (7849)
    ///   and XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE (8689) as baselines.
    /// @param iDeadzone Radius threshold in raw stick units (0-32767)
    void SetDeadzone(int iDeadzone);
    int GetDeadzone() const;

    /// @brief Returns a const reference to the raw XINPUT_GAMEPAD struct
    ///   from the last successful Poll(). Useful for logging or 
    ///   displaying all fields withough going through individual accessors
    const XINPUT_GAMEPAD& GetGamepad() const;

  private:
    /// @brief Applies radial deadzone and rescales a single stick axis
    ///   Called internally by GetLX/LY/RX/RY
    /// @param sAxis Raw axis value from XInput
    /// @param sOther the paired axis (needed for radial magnitude check)
    /// @param iThreshold Deadzone radius to apply
    /// @return Filtered and rescaled value in [-32767, +32767]
    static SHORT ApplyDeadzone(SHORT sAxis, SHORT sOther, int iThreshold);
    
    XINPUT_STATE state_current; //xInput state from last Poll()
    int iPlayer; // Player slot index
    BOOL bConnected;
    int iDeadzone;
};