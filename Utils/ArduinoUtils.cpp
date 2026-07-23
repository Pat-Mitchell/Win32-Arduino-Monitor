/// @file ArduinoUtils.cpp
/// @brief Implementation of shared Arduino utility functions.

#include "ArduinoUtils.h"

// Max frame length: 8 fields * ~10 chars each = ~80 chars
// 128 bytes gives comfortable headroom
static char s_arrFrameBuf[128];
static int s_iFrameLen = 0;
static bool s_bFirstField = true;

float measureVCC() {
  // Point the ADC at the internal 1.1V bandgap refrence.
  // REFS0 = use AVCC as reference voltage
  // MUX bits select teh bandgap as the input channel
  //
  // ATmega2560: bandgap is on MUX4:0 = 11110 (0x1E)
  // ATmega328P: bandgap is on MUX3:0 = 1110  (0x0E)
  #if defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | 0x1E;
  #else
    ADMUX = _BV(REFS0) | 0x0E;
  #endif

  // Wait for the bandgap reference to stabalise
  delay(2);

  // Discard teh first conversion. The ADC needs one cycle to settle
  // on the new MUX selection before the reading is trustworthy
  ADCSRA |= _BV(ADSC);
  while(bit_is_set(ADCSRA, ADSC));

  // Take the atual reading
  ADCSRA |= _BV(ADSC);
  while(bit_is_set(ADCSRA, ADSC));

  // ADC result = (Vbandgap / VCC) * 1023
  // Rearranging: VCC = (Vbandgap * 1023) / ADC
  // Vbandgap is nominally 1.1V; the constant 1125.3 = 1.1 * 1023
  uint8_t low = ADCL;
  uint8_t high = ADCH;
  long lAdc = (high << 8) | low;

  if(lAdc == 0) return 0.0f;

  return 1125.3f / (float)lAdc;
}

int steadyAnalogRead(int iPin) {
  analogRead(iPin); // Dummy read: charges sample-and-hold capacitor
  return analogRead(iPin);
}

void serialBeginFrame() {
  s_arrFrameBuf[0] = '\0';
  s_iFrameLen = 0;
  s_bFirstField = true;
}

void serialSendField(const char* szKey, float fValue, int iDecimalPlaces) {
  // Guard against buffer overrun. Silently drop fields that would overflow
  if(s_iFrameLen >= 120) return;

  // Prepend comma separator for all fields after the first
  if(!s_bFirstField) {
    s_arrFrameBuf[s_iFrameLen++] = ',';
    s_arrFrameBuf[s_iFrameLen] = '\0';
  }
  s_bFirstField = false;

  // Append Key
  int iKeyLen = strln(szKey);
  memcpy(s_arrFrameBuf + s_iFrameLen, szKey, iKeyLen);
  s_iFrameLen += iKeyLen;

  // Appends colon separator
  s_arrFrameBuf[s_iFrameLen++] = ':';

  // Append value using dtostrf (Arduino's float-to-string for AVR)
  // dtostrf(value, min_width, decimal_places, buffer)
  char arrValBuf[16];
  dtostrf(fValue, 1, iDecimalPlaces, arrValBuf);

  int iValLen = strlen(arrValBuf);
  memcpy(s_arrFrameBuf + s_iFrameLen, arrValBuf, iValLen);
  s_iFrameLen += iValLen;

  s_arrFrameBuf[s_iFrameLen] = '\0';
}

void serialEndFrame() {
  Serial.println(s_arrFrameBuf); // println appends '\n'
}