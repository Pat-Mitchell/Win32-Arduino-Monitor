/// @file ArduinoUtils.h
/// @brief Reusable Arduino utility functions
///   Include this file in any sketch that needs VCC measurements,
///   stable ADC reads, or structured serial frame output.
///
/// @note Designed for Uno R3 and Mega 2560
///   measureVCC() uses the internal bandgap reference present in both chips

#pragma once
#include <Arduino.h>

/// @brief  Measures the actual supply voltage by reading the internal
///   1.1V bandgap reference against VCC.
///   Use this instead of assuming VCC = 5V. USB and long cable runs
///   call pull it down by 100-200mV, which skews all ADC derived voltage calculations.
/// @return VCC in volts
float measureVCC();

/// @brief Performs a dummy analogRead folloed by the real one to allow
///   the DC sample-and-hold capacitor to fully charge before sampling.
///   Skipping this causes the first read after a channel switch to
///   reflect the previous channel's voltage
/// @param iPin Analog pin number to read
/// @return ADC reading [0,1023]
int steadyAnalogRead(int iPin);

/// @brief Clears the internal frame buffer. Call once at the start of
///   each transimission before any serialSendField calls.
void serialBeginFrame();

/// @brief Appends a key:value pair to the frame buffer.
///   Multiple fields are separated by commas automatically.
///   e.g. two calls with "V"/4.87 and "I"/0.023 prodice "V:4.87,I:0.023"
/// @param szKey Field key string
/// @param fValue Float value to encode
/// @param iDecimalPlaces Number of decimal places
void serialSendField(const char* szKey, float fValue, int iDecimalPlaces = 2);

/// @brief Transmits the completed frame over Serial followed by '\n'
///   The newline triggers the win32 app's readStringUntil('\n').
void serialEndFrame();