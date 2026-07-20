/// @file omhs_law.ino
/// @brief Reads a voltage dividerr on A0 and transmits V, I, R, P
///        over Serial very 100ms. R_known must match the physical
///        resistor value defined below

/// Known fixed resistor in the voltage divider (Ohms).
/// Change this if using a different value.
/// 100ohm should be the absolute bare minimum before
/// 1/4 watt resistor is damaged, but still not recommended.
const float fR_known = 1000.0; 

/// ADC reference voltage (5V on the Mega)
const float fV_ref = 5.0;

/// ADC max reading (10-bit = 1023)
const float fAdc_max = 1023.0;

void setup() {
  Serial.begin(9600);
  Serial.println("Ohm's Law Visualizer Ready!");
}

void loop() {
  // ──────────── ⋆⋅☆⋅⋆ ──────────────
  // Read ADC and convert to voltage
  //
  // V_out = (D_out / D_max) * V_max
  // ──────────── ⋆⋅☆⋅⋆ ──────────────
  int iAdcRaw = analogRead(A0);
  float fV_node = (iAdcRaw / fAdc_max) * fV_ref;

  // ──────────── ⋆⋅☆⋅⋆ ──────────────
  // Derive I, R_pot, and P from V_node and R_known
  //
  // I = V_node / R_known
  // R_pot = (V_ref - V_node) / I
  // P = I * V_ref
  // ──────────── ⋆⋅☆⋅⋆ ──────────────

  float fI = fV_node / fR_known; // Current through R_known
  float fV_pot = fV_ref - fV_node; // voltage dropped across pot
  
  // Avoid div by 0 -> I = 0
  float fR_pot = (fI > 0.0001) ? (fV_pot / fI) : 0.0; // R = V/I
  float fP = fV_ref * fI; // Total power in circuit

  // ──────────── ⋆⋅☆⋅⋆ ──────────────
  // Transmit as a comma-separated key:value string
  // "V:x.xx,I:x.xx,R:xxxx.x,P:x.xxxx\n"
  // The Win32 parser splits on commas and colons 
  // ──────────── ⋆⋅☆⋅⋆ ──────────────
  Serial.print("V:");  Serial.print(fV_node, 3);
  Serial.print(",I:"); Serial.print(fI,      4);
  Serial.print(",R:"); Serial.print(fR_pot,  1);
  Serial.print(",P:"); Serial.println(fP,      5);

  delay(100);
}
