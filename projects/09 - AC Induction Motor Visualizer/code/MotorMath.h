/// @file MotorMath.h
/// @brief Pure calculation struct for a three-phase induction motor.
///   Sweeps slip from standstill (s = 1) to syncronous speed (s = 0)
///   and produces a normalised torque-speed curve plu annotated
///   key operating points.

#ifndef M_PI
#define M_PI  3.14159265f
#endif

#pragma once
#include <cmath>

static const int CURVE_STEPS = 200;
static const float FULLLOAD_SLIP = 0.05f; // Fixed full-load slip (5%)
// Fixed full load slip for now for simpler math

// ────── ⋆⋅☆⋅⋆ ────────
// Input/Output structs
// ────── ⋆⋅☆⋅⋆ ────────

struct MotorInputs {
  float fV_rms; // Stator phase voltage (Vrms)
  float fFreq_Hz; // Supply frequency (Hz)
  int iPoles; // Total pole count (must be even and obviously more than 0)
  float fR2; // Rotor resistance (Ohms)
  float fX2; // Rotor leakage reactance at standstill (Ohms)
};

struct MotorState {
  float fNs_rpm; // Synchronous speed

  // Mechanical angulat velocity of the rotating field (rad/s)
  // Used by the rotating field animator to adnace theta each tick.
  float fOmega_s;

  // Curve arrays
  // CURVE_STEPS + 1 points sweeping left to right (0RPM to Ns).
  // 0: standstill (s = 1.0, Nr = 0RPM)
  // CURVE_STEPS: synchronous (s = 0.0, Nr = NS RPM)
  // arrTorque is normalised so the peak (breakdown) value = 1.0.
  // The panel scales the y-axis to this maximum by definition.
  float arrNr_rpm[CURVE_STEPS + 1];
  float arrTorque[CURVE_STEPS + 1];

  // Key operating points
  float fT_start; // starting torque
  float fT_breakdown; // breakdown torque = 1.0
  float fSlip_breakdown; // Slip at breakdown torque
  float fNr_breakdown;  // Rotor speed at breakdown (RPM)
  float fT_fullload; // torque at full-load slip (5%)
  float fNr_fullload; // Rotor speed at full-load slip (RPM)

  bool bValid;
};

/// @brief Computes the proportional torque at a given slip value
///   Using the torque equation from the notes md file
///   T = (s * R2) / (R2 ^ 2 + (s * X2) ^ 2)
/// @param fSlip flip in [0, 1]
/// @param fR2 Rotor resistance (Ohms)
/// @param fX2 Rotor standstill leakage reactance (Ohms)
/// @return Proportional torque
static inline float TorqueAtSlip(float fSlip, float fR2, float fX2) {
  float fDenom = fR2 * fR2 + (fSlip * fX2) * (fSlip * fX2);
  if(fDenom < 1e-12f) {
    return 0.0f;
  }
  return (fSlip * fR2) / fDenom;
}

/// @brief Derives the full motor torque-speed characteristic
///   Sweeps slip from 1.0 to 0.0 in CURVE_STEPS equal steps.
///   The resulting torque array is nomalised so its peak value
///   is 1.0.
/// @param rIn Populated MotorInputs
/// @return MotorState with all fields populated
inline MotorState Compute(const MotorInputs& rIn) {
  MotorState s = {};

  // Guards
  if(rIn.fFreq_Hz <= 0.0f || rIn.iPoles < 2 || rIn.iPoles % 2 != 0 || rIn.fR2 <= 0.0f || rIn.fX2 < 0.0f || rIn.fV_rms < 0.0f) {
    s.bValid = false;
    return s;
  }

  s.bValid = true;

  // Synchronous speed
  s.fNs_rpm = 120.0f * rIn.fFreq_Hz / (float)rIn.iPoles;
  s.fOmega_s = 2.0f * M_PI * s.fNs_rpm / 60.0f;

  // Curve sweep
  float fRawMax = 0.0f;
  int iBreakIdx = 0;

  for(int i = 0; i <CURVE_STEPS; i++) {
    float fSlip = 1.0f - (float)i / (float)CURVE_STEPS;
    float fNr = s.fNs_rpm * (1.0f - fSlip);
    float fT = TorqueAtSlip(fSlip, rIn.fR2, rIn.fX2);

    s.arrNr_rpm[i] = fNr;
    s.arrTorque[i] = fT;

    if(fT > fRawMax) {
      fRawMax = fT;
      iBreakIdx = i;
    }
  }

  // Guard against degenerate curves where R2 and X2 produce no torque
  if(fRawMax < 1e-12f) {
    s.bValid = false;
    return s;
  }
  
  // Normalise
  // Divide every value by the peak so arrTorque[iBreakIdx] = 1.0.
  // The panel can then use fTorqueMax = 1.0 as its y-axis ceiling
  //  and all key point values are directly comparable ratios.
  for(int i = 0; i < CURVE_STEPS; i++) {
    s.arrTorque[i] /= fRawMax;
  }

  // starting torque
  s.fT_start = s.arrTorque[0];

  // Breakdown torque: The peak or 1.0 after normalization
  s.fT_breakdown = 1.0f;
  s.fSlip_breakdown = 1.0f - (float)iBreakIdx / (float)CURVE_STEPS;
  s.fNr_breakdown = s.arrNr_rpm[iBreakIdx];

  // Full-load point
  int iFL=(int)((1.0f - FULLLOAD_SLIP) * (float)CURVE_STEPS + 0.5f);
  if(iFL >CURVE_STEPS) {
    iFL = CURVE_STEPS;
  }
  s.fT_fullload = s.arrTorque[iFL];
  s.fNr_fullload = s.arrNr_rpm[iFL];

  return s;
}