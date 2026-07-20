/// @file XInputController.cpp

#include "XInputController.h"
#include <math.h> // sqrtf

XInputController::XInputController(int iPlayerIndex)
  : iPlayer(iPlayerIndex)
  , bConnected(FALSE)
  , iDeadzone(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
{
  ZeroMemory(&state_current, sizeof(XINPUT_STATE));
}

BOOL XInputController::Poll() {
  ZeroMemory(&state_current, sizeof(XINPUT_STATE));
  DWORD dwResult = XInputGetState(iPlayer, &state_current);
  bConnected = (dwResult == ERROR_SUCCESS);
  return bConnected;
}

BOOL XInputController::IsConnected() const {
  return bConnected;
}

BOOL XInputController::IsPressed(WORD wButton) const {
  if(!bConnected) return FALSE;
  return (state_current.Gamepad.wButtons & wButton) != 0;
}

BYTE XInputController::GetLeftTrigger() const {
  if(!bConnected) return 0;
  return state_current.Gamepad.bLeftTrigger;
}

BYTE XInputController::GetRightTrigger() const {
  if(!bConnected) return 0;
  return state_current.Gamepad.bRightTrigger;
}

SHORT XInputController::GetRawLX() const {
  if(!bConnected) return 0;
  return state_current.Gamepad.sThumbLX;
}

SHORT XInputController::GetRawLY() const {
  if(!bConnected) return 0;
  return state_current.Gamepad.sThumbLY;
}

SHORT XInputController::GetRawRX() const {
  if(!bConnected) return 0;
  return state_current.Gamepad.sThumbRX;
}

SHORT XInputController::GetRawRY() const {
  if(!bConnected) return 0;
  return state_current.Gamepad.sThumbRY;
}

SHORT XInputController::GetLX() const {
  if(!bConnected) return 0;
  return ApplyDeadzone(state_current.Gamepad.sThumbLX, state_current.Gamepad.sThumbLY, iDeadzone);
}

SHORT XInputController::GetLY() const {
  if(!bConnected) return 0;
  return ApplyDeadzone(state_current.Gamepad.sThumbLY, state_current.Gamepad.sThumbLX, iDeadzone);
}

SHORT XInputController::GetRX() const {
  if(!bConnected) return 0;
  return ApplyDeadzone(state_current.Gamepad.sThumbRX, state_current.Gamepad.sThumbRY, iDeadzone);
}

SHORT XInputController::GetRY() const {
  if(!bConnected) return 0;
  return ApplyDeadzone(state_current.Gamepad.sThumbRY, state_current.Gamepad.sThumbRX, iDeadzone);
}

void XInputController::SetDeadzone(int iNewDeadzone) {
  iDeadzone = iNewDeadzone;
}

int XInputController::GetDeadzone() const {
  return iDeadzone;
}

const XINPUT_GAMEPAD& XInputController::GetGamepad() const {
  return state_current.Gamepad;
}

SHORT XInputController::ApplyDeadzone(SHORT sAxis, SHORT sOther, int iThreshold) {
  // Radial deadzone: test the 2D magnitude of (sAxis, sOther) against the threashold
  //   rather than clamping each axis independently. This keeps the stick
  //   dead at true center instead of producing a square dead zone.
  float fMagnitude = sqrtf((float)sAxis * (float)sAxis + (float)sOther * (float)sOther);

  if(fMagnitude < (float)iThreshold) return 0;

  // Rescale so the output starts at 0 at the deadzone edge rather than jumping
  //   from 0 to iThreshold on first contact
  float fMax = 32767.0f;
  float fNormAxis = (float)sAxis / fMagnitude; // Direction component
  float fScaled = fNormAxis * ((fMagnitude - iThreshold) / (fMax - iThreshold)) * fMax; // Rescaled magnitude

  // Clamp to valid SHORT range
  if(fScaled > 32767.0f) fScaled = 32767.0f;
  if(fScaled < -32767.0f) fScaled = -32767.0f;

  return (SHORT)fScaled;
}