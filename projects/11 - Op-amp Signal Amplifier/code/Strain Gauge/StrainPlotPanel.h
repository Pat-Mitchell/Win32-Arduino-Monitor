/// @file StrainPlotPanel.h
/// @brief Scrolling time-vs-microstrain plot.
///   Negative values shown below the zero line (compression)
///   Positive values shown above the zero line (tension)

// ────── ⋆⋅☆⋅⋆ ────────
#pragma once

#include "../../../../pch.h"
#include "../../../../Utils/Utils.h"

#define MAX_SAMPLES 512

struct StrainPlotPanel {
  float arrTime_ms[MAX_SAMPLES];
  float arrStrain[MAX_SAMPLES];
  int iCount;
  int iPadL, iPadT, iPadR, iPadB;
  RECT rect_bounds;

  void Init(int iX, int iY, int iW, int iH);
  void Clear();
  void AddSamples(float fTime_ms, float fStrain_ue);

  float GetYMax() const;
  int MapX(int iIdx) const;
  int MapY(float fVal, float fYmin, float fYmax) const;
  void Draw(HDC hdc) const;
};