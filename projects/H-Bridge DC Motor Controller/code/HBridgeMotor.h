/// @file HBridgeMotor.h
/// @brief H-Bridge DC motor controller
///   Bidirectional speed control with current history plot and stall detection

#pragma once

#include "../../../pch.h"
#include "../../../Utils/Utils.h"
#include "../../../wrappers/win32Wrappers/Window.h"
#include "../../../wrappers/win32Wrappers/Button.h"
#include "../../../wrappers/win32Wrappers/Label.h"
#include "../../../wrappers/win32Wrappers/TextInput.h"
#include "../../../wrappers/win32Wrappers/ComboBox.h"
#include "../../../wrappers/win32Wrappers/Trackbar.h"
#include "../../../wrappers/serialPortWrappers/SerialPort.h" 

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

// ────── ⋆⋅☆⋅⋆ ────────
// Control IDs
// ────── ⋆⋅☆⋅⋆ ────────
#define ID_COMBO_PORT 101
#define ID_BTN_CONNECT 102
#define ID_BTN_DISCONNECT 103
#define ID_TRACKBAR_SPEED 104
#define ID_BTN_BRAKE 105
#define ID_BTN_COAST 106
#define ID_BTN_CLEARSTALL 107
#define ID_EDIT_VPIN 108
#define ID_BTN_SETVPIN 109
#define ID_BTN_EXPORT 110
#define ID_TIMER_POLL 1

#define POLL_MS 50
#define READ_BUF 512
#define MAX_SAMPLES 512
#define STALL_MA 1200.0f // Current threashold for stall display line

// ────── ⋆⋅☆⋅⋆ ────────
// Current Plot Panel
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Scrolling time-vs-current plot.
///   When full, oldestsamples are discarded from the left.
///   Y axis auto-scales above stall threshold

struct CurrentPlotPanel {
  float arrTime[MAX_SAMPLES]; // Timestamps in ms
  float arrCurr[MAX_SAMPLES]; // Current readings in mA
  int iCount;
  int iPadL, iPadT, iPadR, iPadB;
  RECT rect_bounds;

  void Init(int iX, int iY, int iW, int iH);
  void Clear();

  // Appends a sample. Scrolls left when buffer is full
  void AddSample(float fTime_ms, float fCurr_mA);

  float GetYMax() const;
  int MapXByIdx(int i) const;
  int MapY(float fVal, float fYmax) const;

  void Draw(HDC hdc) const;
};