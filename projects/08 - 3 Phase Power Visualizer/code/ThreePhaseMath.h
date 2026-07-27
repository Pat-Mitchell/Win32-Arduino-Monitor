/// @file ThreePhaseMath.h
/// @brief Pure Calculation struct for a balanced three-phase system.
///   Supports both stary and delta configurations and both phase sequences.

#pragma once
#include <cmath>

#ifndef M_PI
#define M_PI  3.14159265
#endif

// Enums
enum class Config { Star, Delta };
enum class PhaseSeq { ABC, ACB };

// Input/Output structs
struct ThreePhaseInputs {
  float fV_phase_rms; // per-phase RMS voltage (V)
  float fFreq_Hz; // Supply frequency (Hz)
  float fR_load; // Per-pahase resistive load (Ohms)
  Config eConfig; // Star or Delta
  PhaseSeq eSeq; // ABC (positive) or ACB (negative)
};

struct ThreePhaseState {
  // ────── ⋆⋅☆⋅⋆ ────────
  // Voltages
  // ────── ⋆⋅☆⋅⋆ ────────
  float fV_phase_rms; // Echo of input
  float fV_phase_peak; // V_phase_rms * sqrt(2). Waveform amplitude
  float fV_line_rms; // Star: sqrt(3)*V_phase; Delta: V_phase;

  // ────── ⋆⋅☆⋅⋆ ────────
  // Phase angle offsets (radians)
  // ────── ⋆⋅☆⋅⋆ ────────
  // A is always referenced at 0
  // B and C swap beteen ABC and ACB
  //   ABC: A = 0; B = -2pi/3; C = -4pi/3
  //   ACB: A = 0; B = -4pi/3; C = -2pi/3
  float fOffset_A_rad;
  float fOffset_B_rad;
  float fOffset_C_rad;

  // ────── ⋆⋅☆⋅⋆ ────────
  // Current
  // ────── ⋆⋅☆⋅⋆ ────────
  // Star: I_line = I_phase = V_phase / R
  // Delta: I_phase = V_phase / R; I_line = sqrt(3) * I_phase
  float fI_phase_rms;
  float fI_line_rms;

  // ────── ⋆⋅☆⋅⋆ ────────
  // Power
  // ────── ⋆⋅☆⋅⋆ ────────
  // Purely resistive load so cos(phi) = 1.0 always
  // P_total = 3 * V_phase * I_phase
  //         = sqrt(3) * V_line * I_line 
  float fP_total;
  float fPowerFactor; // 1.0 for resistive load. Motor is in resonance

  // ────── ⋆⋅☆⋅⋆ ────────
  // Star only
  // ────── ⋆⋅☆⋅⋆ ────────
  // Neutral current is laways 0 A in a balanced load
  // Stored explicitly so the readout strip can display it
  float fI_neutral;

  // ────── ⋆⋅☆⋅⋆ ────────
  // Metadata
  // ────── ⋆⋅☆⋅⋆ ────────
  Config eConfig;
  PhaseSeq eSeq;
  bool bValid;
};

// ────── ⋆⋅☆⋅⋆ ────────
// Compute
// ────── ⋆⋅☆⋅⋆ ────────


/// @brief Derives all three phase quantities from a set of balanced inputs
///   Phase offsets are written into the state so both panels can read
///   them directly.
/// @param rIn Populated threePhaseInputs struct
/// @return ThreePhaseState with all computed values
inline ThreePhaseState Compute(const ThreePhaseInputs& rIn) {
  ThreePhaseState s = {};

  // Guards
  //   Frequency, R, and V must be positive
  if(rIn.fFreq_Hz <= 0.0f || rIn.fR_load <= 0.0f || rIn.fV_phase_rms <= 0.0f) {
    s.bValid = false;
    return s;
  }

  s.bValid = true;
  s.eConfig = rIn.eConfig;
  s.eSeq = rIn.eSeq;

  // Voltage
  s.fV_phase_rms = rIn.fV_phase_rms;
  s.fV_phase_peak = rIn.fV_phase_rms * std::sqrt(2.0f);

  if(rIn.eConfig == Config::Star) {
    s.fV_line_rms = std::sqrt(3.0f) * rIn.fV_phase_rms; // Star: V_line == V_phase * sqrt(3)
  } else {
    s.fV_line_rms = rIn.fV_phase_rms; // Delta: V_line == V_phase
  }

  // Phase offsets
  //   Defined in radians. 
  //   A is always referenced at 0
  //   B and C swap beteen ABC and ACB
  //   The waveform equations and phasor angles both flow from these values
  const float fTwoThirdsPi = 2.0f * M_PI / 3.0f; // 120 deg in radians
  const float fFourThirdsPi = 4.0f * M_PI / 3.0f; // 240 deg in radians

  s.fOffset_A_rad = 0.0f;

  if(rIn.eSeq == PhaseSeq::ABC) {
    s.fOffset_B_rad = -fTwoThirdsPi;
    s.fOffset_C_rad = -fFourThirdsPi;
  } else {
    // ACB
    // Reverse the order of the phasor diagram
    s.fOffset_B_rad = -fFourThirdsPi;
    s.fOffset_C_rad = -fTwoThirdsPi;
  }

  // Current
  // Per-phase current is always V_phase / R regardless of config
  // Line current changes
  s.fI_phase_rms = rIn.fV_phase_rms / rIn.fR_load;

  if(rIn.eConfig == Config::Star) {
    s.fI_line_rms = s.fI_phase_rms; // Star: I_line == I_phase
  } else {
    s.fI_line_rms = std::sqrt(3.0f) * s.fI_phase_rms; // Delta: I_line == sqrt(3) * I_phase
  }

  // Power
  s.fPowerFactor = 1.0f; // Resistive load only in the sim.
  s.fP_total = 3.0f * rIn.fV_phase_rms * s.fI_phase_rms;

  // Neutral current
  //   In a balanced star load the three currents are equal in magnitude
  //   and 120 deg apart, so they sum to exactly zero by the same vector
  //   identity that eliminates the third wire in balanced transimission.
  //   Delta has no neutral by topology so this is always 0 in both cases
  //   This sim assumes ideal circumstances, so neutral will always be 0.0.
  s.fI_neutral = 0.0f;

  return s;
}