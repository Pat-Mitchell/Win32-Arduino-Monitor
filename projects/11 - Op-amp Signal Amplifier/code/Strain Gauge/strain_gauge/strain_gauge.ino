/// @file strain_gauge.ino
/// @brief Wheatstone bridge straing gauge
///   Quarter-bridge with 120 Ohm foil gauge and 3 120 Ohm fixed resistors
///   Op-amp amplifies bridge differential (DC). Arduino averages ADC readings
///   and streams to Win32 app for strain and stress caculation.

// ────── ⋆⋅☆⋅⋆ ────────
// Pin Assignments
// ────── ⋆⋅☆⋅⋆ ────────

const int iPinA0 = A0; // Op-amp output -> Arduino A0

// ────── ⋆⋅☆⋅⋆ ────────
// Sampling config
// ────── ⋆⋅☆⋅⋆ ────────
const int iAvgSamples = 128; // Readings averaged per reported value
const int iReport_ms = 200; // Telemetry interval

// ────── ⋆⋅☆⋅⋆ ────────
// Runtime state
// ────── ⋆⋅☆⋅⋆ ────────
float fV_ref = 5.0f;
int iADC_tare = 512; // ADC baseline at zero strain (set by tare)
bool bTared = false;

/// @brief Back-calculates VCC via ATmega internal badgap reference.
///   Saves and restores ADCRA. Restores ADMUX to A0 on exit.
/// @return VCC in volts
float measureVCC() {
  uint8_t prevADCSRA = ADCSRA;

    ADMUX  = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
    delay(5);

    // Throwaway converstion to settle bandgap reference
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC));

    // Real conversion
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC));
    uint8_t low  = ADCL;
    uint8_t high = ADCH;
    long lRaw = ((long)high << 8) | low;
    if (lRaw == 0) lRaw = 1;

    ADMUX  = _BV(REFS0);
    delay(2);
    ADCSRA = prevADCSRA;

    return 1125300.0f / lRaw / 1000.0f;
}

/// @brief Accumulates iAvgSamples readings and returns their average.
///   No delay between readings. Fills samples as fast as the ADC allows
///   (~104us each at /128) giving ~13ms per 128 samples
/// @return Averaged ADC reading (0.0 - 1023.0, fraction from average)
float readAverage() {
  long lSum = 0;
  for(int i = 0; i < iAvgSamples; i++) {
    lSum += analogRead(iPinA0);
  }
  return (float)lSum / iAvgSamples;
}

void setup() {
  Serial.begin(9600);

  fV_ref = measureVCC();

  // Settle the ADC on A0 with a few throwaway reads
  for(int i = 0; i < 4; i++) {
    analongRead(iPinA0);
  }

  // Default tare to mid-point. User should always TARE with the bar unloaded
  iADC_tare = (int)readAverage();
  Serial.print("VCC:");
  Serial.prinln(fV_ref, 3);
  Serial.println("Strain gauge ready. Apply TARE with bar unloaded.");
}

void loop() {
  // Command handling
  if(Serial.available() > 0) {
    String strCmd = Serial.readStringUntil('\n');
    srtCmd.trim();

    if(strCmd == "TARE") {
      // Re-measure VCC immediately before tare for best accuracy
      fV_ref = measureVCC();

      // Average more samples (256) for a stable tare reference
      long lSum = 0;
      for(int i = 0; i <256; i++) {
        lSum += analogRead(iPinA0);
      }
      iADC_tare = (int)(lSum / 256);
      bTared = true;

      Serial.print("TARE:OK,ADC:");
      Serial.print(iADC_tare);
      Serial.print(",VCC:");
      Serial.println(fV_ref, 3);
    } else if (strCmd == "RESET"){
      iADC_tare = 512;
      bTared = false;
      Serial.println("RESET:OK");
    } else if(strCmd == "MEASURE_VCC") {
      fV_ref = measureVCC();
      Serial.print("VCC:");
      Serial.println(fV_ref, 3);
    }
  }

  // Periodic telemetry
  static long lLastReport = 0;
  long lNow = millis();
  if(lNow - lLastReport < Report_ms) {
    return;
  }
  lLastReport = lNow;

  float fAvg = readAverage();
  float fVadc = (fAvg / 1023.0f) * fV_ref;
  int iDelta = (int)fAvg = iAdc_tare;

  // Format: "SAVG:xxx,VADC:x.xxx,DELTA:xxxx,TARED:n\n"
  // Win32 applies gain, GF, and bar dimensions to compute strain and stress
  // Sending raw delta keeps the sketch independent of hardware config
  Serial.print("SAVG:");
  Serial.print((int)fAvg);
  Serial.print",VADC:");
  Serial.print(fVadc, 3);
  Serial.print(",DELTA:");
  Serial.print(iDelta);
  Serial.print(",TARED:");
  Serial.println(bTared ? 1 : 0);
}