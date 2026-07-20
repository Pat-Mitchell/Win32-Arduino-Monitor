/// @file PhasorMath.h
#pragma once
#include <cmath>

struct PhasorInputs {
  float fR; // Resistance (Ohms)
  float fL_mH; // Inductance (millihenries)
  float fC_uF; // Capacitance (microfarads)
  float fFreq_Hz; // Supply frequency (Hz)
  float fV_rms; // Source RMS voltage (V)
};

struct PhasorState {
  // Reactances
  float fXL; // Inductive reactance (Ohms)
  float fXC; // Capacitive reactance (Ohms)
  float fX; // Net reactance = X_L - X_C (Ohms, signed)
  float fZ; // Impedance magnitude (Ohms)

  // Phase
  float fPhi_rad; // Phase angle (radians) positive = inductance, negative = capacitive

  // Current
  float fI_rms; // RMS current (A)

  // Component voltages
  // Magnitude only; direction is implicit from convention:
  // V_R: Colinear w/ I (0 deg)
  // V_L: 90 deg ahead of I (+j axis)
  // V_C: 90 deg behind I (-j axis)
  // VTotal: vector sum at angle fPhi_rad
  float fVR;
  float fVL;
  float fVC;
  float fVTotal;

  // Power triangle
  float fP; // Real power (W)
  float fQ; // Reactive power (VAR, signed: +inductive, -capacitive)
  float fS; // Apparent power (VA)
  float fPowerFactor; // cos(phi), range [0, 1]

  // State
  bool bValid; // False if inputs produce a degenerate circuit
};

/// @brief Derives all AC phasor quantities from a set of series RLC inputs.
///   Uses the current-reference convention: I lies along the +x axis
///   at 0 degrees. V_R is colinear with I; V_L points to +90 deg;
///   V_C points to -90 deg; V_Total is the vector sum angle fPhi_rad.
/// @param rIn Populated PhasorInputs struct.
/// @return PhasorState with all computed values. Always check bValid before
//   passing the result to draw function.
inline PhasorState Compute(const PhasorInputs& rIn) {
  PhasorState s = {};

  // Guard: f <= 0 means DC or invalid. At DC the capacitor is an Open
  //   circuit (X_C = inf) and the series current is zero; the math degenerates.
  // Enforce f > 0 on the input side and reject here as a safety net.
  if(rIn.fFreq_Hz <= 0.0f || rIn.fV_rms <= 0.0f) {
    s.bValid = false;
    return s;
  }

  const float fPi = 3.14159265358979f;
  const float fOmega = 2.0f * fPi * rIn.fFreq_Hz; // rad/s

  // ────── ⋆⋅☆⋅⋆ ────────
  // Reactances
  // ────── ⋆⋅☆⋅⋆ ────────

  // X_L = \omega * L. Convert mH -> H
  // At f > 0 with L = 0, X_L = 0 (ideal wire)
  s.fXL = fOmega * (rIn.fL_mH / 1000.0f);

  // X_C = 1 / (\omega * C). Convert uF -> F.
  // If C = 0 the capacitor is absent; treat as open circuit (X_C = 0 means
  //   the C branch is simply not in the loop). The app should hide the C
  //   field or show "no capacitor" when C = 0.
  if(rIn.fC_uF > 0.0f) {
    s.fXC = 1.0f / (fOmega * (rIn.fC_uF / 1000000.0f));
  } else {
    s.fXC = 0.0f;
  }

  s.fX = s.fXL - s.fXC; // Positive = inductive, negative = capacitive

  // ────── ⋆⋅☆⋅⋆ ────────
  // Inpedance
  // ────── ⋆⋅☆⋅⋆ ────────
  s.fZ = sqrtf(rIn.fR * rIn.fR + s.fX * s.fX);

  // Guard: Z = 0 is a theorectical short circuit (R = 0, X = 0)
  // Infinite current -> flag invalid
  if(s.fZ < 1e-6f) {
    s.bValid = false;
    return s;
  }

  s.bValid = true;

  // ────── ⋆⋅☆⋅⋆ ────────
  // Phase Angle
  // atan2f(X, R) gives the signed angle in (-pi / 2, +pi / 2) for R >= 0.
  // Preferred over atanf(X / R) because it handles R = 0 without dividing by zero.
  // ────── ⋆⋅☆⋅⋆ ────────
  s.fPhi_rad = atan2f(s.fX, rIn.fR);

  // ────── ⋆⋅☆⋅⋆ ────────
  // Current
  // ────── ⋆⋅☆⋅⋆ ────────
  s.fI_rms = rIn.fV_rms / s.fZ;

  // ────── ⋆⋅☆⋅⋆ ────────
  // Component voltages
  // ────── ⋆⋅☆⋅⋆ ────────
  s.fVR = s.fI_rms * rIn.fR;
  s.fVL = s.fI_rms * s.fXL;
  s.fVC = s.fI_rms * s.fXC;
  s.fVTotal = rIn.fV_rms; // Source voltage

  // ────── ⋆⋅☆⋅⋆ ────────
  // Power triangle
  // ────── ⋆⋅☆⋅⋆ ────────
  s.fP = s.fI_rms * s.fI_rms * rIn.fR; // I * I * R
  s.fQ = s.fI_rms * s.fI_rms * s.fX; // I * I * X (signed)
  s.fS = rIn.fV_rms * s.fI_rms; // VI
  s.fPowerFactor = cosf(s.fPhi_rad); // Range [0, 1] for phi in (-pi/2, pi/2)

  return s;
}