/// @file wheatstone.ino
/// @brief Wheatstone Bridge & Thermistor Temperature Logger.
///   Reads V_a (A0) and V_b (A1) from a Wheatstone bridge with NTC
///   thermistor as R_x. Calculates V_diff, R_therm, and temperature
///   via the Beta wquation. Implements a calibration for VCC and 
///   double analogRead to lower % error from theorectical calcs

// ────── ⋆⋅☆⋅⋆ ────────
// Componenet values 
// ────── ⋆⋅☆⋅⋆ ────────

// *Measured* values of resistors (10k is the factory reported value)
const float fR1 = 9930.0;
const float fR2 = 9970.0;
const float fR3 = 9980.0;

// Thermistor values
const float fRX = 10000.0; // Measured resistance @ room temp
const float fT0_K = 298.15; // Reference temp 298.15K (25C = room temp)
const float fBeta = 3950.0; // Factory reported Beta value

// Ms between serial transmission
const int iReport_ms = 500;

// ────── ⋆⋅☆⋅⋆ ────────
// Runtime state
// ────── ⋆⋅☆⋅⋆ ────────
float fV_ref = 5.0; // Actual VCC. Measured at startup and updated
float fTemp_offset = 0; // Calibration offset in C
bool bCalibrated = false;

/// @brief Back-calculates actual VCC by reading the ATmega's internal 1.1V
///   bandgap reference. More accurate than assuming a perfect 5.0V USB
///   supply. Called at startup and periodically to compensate for drift.
/// @returns VCC in volts.
float measureVCC() {
  // Select VCC as ADC reference, route internal 1.1V bandgap to ADC input.
  // MUX bits 4:0 = 11110 selects the 1.1V internal reference channel.
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(5); // Let bandgap reference settle

  ADCSRA |= _BV(ADSC); // Start conversion
  while(bit_is_set(ADCSRA, ADSC)); // Block until complete

  long lRaw = ADCL;
  lRaw |= ADCH << 8;

  // Rearranged from: V_bandgap = VCC * lRaw / 1023
  // -> VCC = 1.1 * 1023 / lRaw (1125300 = 1100 * 1023 in mV)
  return 1125300.0 / lRaw / 1000.0;
}

/// @brief Reads an analog pin twice, discarding the first result.
///   The first read charges the ADC sample-and-hold capacitor from the 
///   previous channel. The second read is accurate.
/// @param iPin Analog pin number.
/// @return Settled ADC reading (0-1023)
int doubleRead(int iPin) {
  analogRead(iPin); // Discard. Settles sample capacitor
  return analogRead(iPin); // More accurate reading
}

/// @brief Converts raw bridge readings to temperature using the Beta equation.
///   R_therm is derived from V_b (voltage divider rearrangement), then
///   converted to Kelvin via the Beta approximation.
/// @param fV_b Voltage at the NTC midpoint (V_b)
/// @return Temperature in C with calibration offset applied
float calcTemp(float fV_b) {
  // Guard against division by zero if V_b saturates near VCC
  if(fV_ref - fV_b < 0.001) return -999.0;

  // Rearranged voltage divider: R_term = R3 * V-b / (V_cc - V_b)
  float fR_therm = fR3 * fV_b / (fV_ref - fV_b);

  if(fR_therm <= 0.0) return -999.0;

  // Beta equation: 1/T = 1/t0 + (1/Beta) * ln(R/R0)
  float fInvT = (1.0 / fT0_K) + (1.0 / fBeta) * log(fR_therm / fRX);
  float fTemp_K = 1.0 / fInvT;
  float fTemp_C = (fTemp_K - 273.15) + fTemp_offset;

  return fTemp_C;
}

void setup() {
  Serial.begin(9600);
  fV_ref = measureVCC();

  Serial.print("VCC:"); Serial.println(fV_ref, 3);
  Serial.println("Wheatstone Bridge ready.");
}

void loop() {
  // ────── ⋆⋅☆⋅⋆ ────────
  // Command handling
  // ────── ⋆⋅☆⋅⋆ ────────

  if(Serial.available() > 0) {
    String strCmd = Serial.readStringUntil('\n');
    strCmd.trim();

    if(strCmd == "CALIBRATE") {
      // Re-measure VCC immediately before calibrating
      fV_ref = measureVCC();

      // Average 10 readings for a stable calibration point
      float fSum = 0.0;
      for(int i = 0; i < 10; i++) {
        float fV_b = (doubleRead(A1) / 1023.0) * fV_ref;
        fSum += calcTemp(fV_b);
        delay(50);
      }
      float fTemp_now = fSum / 10.0;

      // Store offset assuming true temperature is 25C at calibration.
      // If a reference termometer is available, replace 25.0 with the
      // known ambient temperature for higher accuracy.
      fTemp_offset = 20.0 - (fTemp_now - fTemp_offset);
      bCalibrated = true;

      Serial.print("CAL:OK,OFFSET:");
      Serial.println(fTemp_offset, 3);
    } else if(strCmd == "RESET") {
      fTemp_offset = 0.0;
      bCalibrated = false;
      Serial.println("RESET:OK");
    } else if(strCmd == "MEASURE_VCC") {
      fV_ref = measureVCC();
      Serial.print("VCC:");
      Serial.println(fV_ref, 3);
    }
  }

  // ────── ⋆⋅☆⋅⋆ ────────
  // Periodic readout
  // ────── ⋆⋅☆⋅⋆ ────────
  static long lLastReport = 0;
  static long lLastVCC = -30001; // Force VCC update on first tick
  long lNow = millis();

  if(lNow - lLastReport < iReport_ms) return;
  lLastReport = lNow;

  // Re-measure VCC every 30 seconds to compensate for USB power drift
  if(lNow - lLastVCC > 30000) {
    fV_ref = measureVCC();
    lLastVCC = lNow;
  }

  // Read boath bridge midpoints with double-read on each
  float fV_b = (doubleRead(A1) / 1023.0) * fV_ref;  
  // float fV_a = fV_ref * fR2 / (fR1 + fR2);

  // This line is most accurate at the calibration temperature
  // Using measured resistors is more accurate at wider temp ranges
  float fV_a = (doubleRead(A0) / 1023.0) * fV_ref;

  float fV_diff = fV_a - fV_b;
  float fR_therm = (fV_ref - fV_b > 0.001) ? fR3 * fV_b / (fV_ref - fV_b) : 0.0;
  float fTemp_C = calcTemp(fV_b);

  // Format: "VA:x.xxx,VB:x.xxx,VDIFF:x.xxx,RTHERM:xxxxx.x,TEMP:xx.xx,CAL:n\n"
  Serial.print("VA:"); Serial.print(fV_a, 3);
  Serial.print(",VB:"); Serial.print(fV_b, 3);
  Serial.print(",VDIFF:"); Serial.print (fV_diff, 3);
  Serial.print(",RTHERM:"); Serial.print(fR_therm, 1);
  Serial.print(",TEMP:"); Serial.print(fTemp_C, 2);
  Serial.print(",CAL:"); Serial.println(bCalibrated ? 1: 0);
}