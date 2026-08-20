/// @file oscilloscope.ino
/// @brief Op-Amp Signal Amplifier Oscilloscope
///   Fast ADC sampling via prescaler /16 (~76kHz). Triggered and
///   auto-trigger frame capture. Streams frames at 115200 baud.

// ────── ⋆⋅☆⋅⋆ ────────
// Config
// ────── ⋆⋅☆⋅⋆ ────────
#define BAUD_RATE 115200
#define MAX_SAMPLES 512

const int iPinSignal = A0;

// ────── ⋆⋅☆⋅⋆ ────────
// Adjustable vis serial commands
// ────── ⋆⋅☆⋅⋆ ────────
int iSamples = 256; // Samples per frame (64-512)
int iTrigLevel = 360; // ADC trigger threshold (0-716, 360 = ~1.76V)
bool bTrigAuto = true; // TRUE = free-run, FLASE = edge triggered
bool bRisingEdge = true; // TRUE = rising edge, FALSE = falling edge
float fV_ref = 5.0f // Actual VCC updated via measure VCC
float fGain = 10.0f; // Hardware op-amp gain

// ────── ⋆⋅☆⋅⋆ ────────
// Sample buffer
int arrBuf[MAX_SAMPLES];

// ────── ⋆⋅☆⋅⋆ ────────
// ADC and VCC utilities
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Back-calculates VCC via internal bandgap reference 
/// @return VCC in volts
float measureVCC() {
  uint8_t prevADCSRA = ADCSRA;

  ADMUX  = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
  delay(5);

  // Throwaway conversion. Settles bandgap reference
  ADCSRA |= _BV(ADSC);
  while(bit_is_set(ADCSRA, ADSC));

  // Real conversion
  ADCSRA |= _BV(ADSC);
  while(bit_is_set(ADCSRA, ADSC));
  uint8_t low = ADCL;
  uint8_t high = ADCH;
  long lRaw = ((long)high << 8) | low;
  if(lRaw == 0) lRaw = 1;

  // Restore ADMUX to A0 and original prescaler
  ADMUX = _BV(REFS0);
  delay(2);
  ADCSRA = revADCSRA;
}

/// @brief Sets ADC prescaler to /16 for ~76kHz sample rate.
///   ADC clock = 16MHz / 16 = 1MHz.
///   Each conversion = 13 ADC cycles = ~13microseconds.
///   at /16, effective resolution drops to ~8-9 stable bits.
///   Acceptable for audio oscilloscope. Not for precision DC measurements
void setupADC() {
  // Clear lower 3 bits (prescaler, set ADPS2 only = /16)
  ADCSRA = (ADCSRA & ~0x07) | _BV(ADPS2);
}

// ────── ⋆⋅☆⋅⋆ ────────
// Frame capture
// ────── ⋆⋅☆⋅⋆ ────────

/// @brief Fills arrBuf with iSamples readings from A0 as fast as possible
///   Uses single analogRead
void collectFrame() {
  for(int i = 0; i < iSamples; i++) {
    arrBuf[i] = analogRead(iPinSignal);
  }
}

/// @brief Scans the input signal for a rising or falling edge crossing
///   iTrigLevel. Returns TRUE if trigger was found within the timeout.
///   Returns FALSE on timeout. Caller sends the frame anyway (auto-fallback)
///   Two-phase search:
///    1: Wait for signal to be on the "wrong" side of threshold
///    2: Wait for signal to cross to the "right" side
///   This prevents falsely triggering on a signal already past the edge
/// @return TRUE if edge found, FALSE if timeout
bool waitForTrigger() {
  const unsigned long ulTimeout_ms = 300;
  unsigned long ulStart = millis();

  if(bRisingEdge) {
    // Phase 1: Wait for signal
    while(analogRead(iPinSignal) >= iTrigLevel) {
      if(millis() - ulStart > ulTimeout_ms) {
        return false;
      }
    }
    // Phase 2: Wait for signal to cross upward
    while(analogRead(iPinSignal) < iTrigLevel) {
      if(millis() - ulStart > ulTimeout_ms) {
        return false;
      }
    }
  } else {
    // Phase 1:
    while(analogRead(iPinSignal) <= iTrigLevel) {
      if(millis() - ulStart > ulTimeout_ms) {
        return false;
      }
    }
    // Phase 2: Wait for signal to cross downward
    while(analogRead(iPinSignal) > iTrigLevel) {
      if(millis() - ulStart > uTimeout_ms) {
        return false;
      }
    }
  }

  return true;
}

/// @brief Transmits a complete captured frame over Serial
///
/// Format: Header line:
///   "FRAME:n,VCC:x.xxx,GAIN:x,TRIG:n,TRIG_OK:n\n"
///
/// Format: Data line (all samples on one line, comma separated):
///   "v1,v2,v3,...,vn\n"
///
/// Sending all samples on one line minimises newline overhead and
///   simplifies the Win32 line-buffer parser (one \n = one complete dataset
///   to parse and render).
///
/// @param bTriggered TRUE if a valid edge trigger was found for this frame.
void sendFrame(bool bTriggered) {
  Serial.print("FRAME:");
  Serial.print(iSamples);
  Serial.print(",VCC:");
  Serial.print(fV_ref, 3);
  Serial.print(",GAIN:");
  Serial.print((int)fGain);
  Serial.print(",Trig:");
  Serial.print(iTrigLevel);
  Serial.print(",TRIG_OK:");
  Serial.println(bTriggered ? 1 : 0);

  // All sample values on a single comma-separated line
  for(int i = 0; i < iSamples; i++) {
    Serial.print(arrBuf[i]);
    if(i < iSamples - 1) {
      Serial.print(',');
    }
  }
  Serial.println();
}





















