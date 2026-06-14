/// @file lrc_resonance.ino
/// @brief LRC Resonance Detector.
///   Sweeps PWM frequency using Timer 1 phase-correct mode on pin 11
///   Measures peak voltage across R_sense at each step and streams
///   data to the Win32 app. 

float fV_ref = 5.0; // Actual VCC. Measured and overwritten at startup
float fV_pin = 4.85; // Actual PWM pin HIGH voltage. Measure w/ multimeter

const int iSamples = 128; // ADC samples taken per frequency step for peak detection.
const int iSettle_ms = 10; // Settle time after each freqncy change in milliseconds.

bool bSweepActive = false;
bool bSweepStop = false; 

/// @brief Back-calculates VCC via ATmega internal bandgap reference.
/// @return VCC in volts.
float measureVCC() {
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(5);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  long lRaw = ADCL | (ADCH << 8);

  // Restore ADMUX to A0 with VCC reference before returning
  ADMUX = _BV(REFS0);
  delay(2);

  return 1125300.0 / lRaw / 1000.0;
}

/// @brief Reads analog pin twice, discarding the first.
///   The first read charges the ADC sample-and-hold capacitor from the 
///   previous channel. The second read is accurate.
/// @param iPin Analog pin number
/// @return Settled ADC reading
int doubleRead(int iPin) {
  analogRead(iPin);
  return analogRead(iPin);
}

/// @brief Configures Timer 1 for phase-correct PWM moe 10 (ICR1 = TOP)
///   on pin 11. Phase-correct chosen over Fast OWM because symmetic pulses
///   reduce harmonic distortion in the LRC circuit. Gives a cleaner
///   frequency response curve.
///
///   Register config:
///      TCCR1A: COM1A1 + WGM11
///        COM1A1 = non-inverting (clear on up-count, set on down-count)
///        WGM11  = part of mode 10 (phase-correct, ICR1 = TOP)
///      TCCR1B: WGM13 + CS10
///        WGM13  = part of mode 10
///        CS10   = prescaler N = 1 (no division, full 16 MHz clock)
///
///   Frequency formula: f = F_CPU / (2 * N * ICR1)
///   Duty cycle 50%: OCR1A = ICR1 / 2
void setupTimer1() {
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(CS10);
  ICR1 = 160; // Default 50 kHz: 16,000,000 / (2 * 1 * 160) = 50,000 Hz
  OCR1A = 80; // 50% duty: OCR1A = ICR1 / 2
  pinMode(11, OUTPUT);
}

/// @brief Sets Timer 1 PWM frequency on pin 11 and maintains 50% duty.
///   Recalculates ICR1 from target frequency, clamps to valid range.
/// @param lFreq_Hz Target freqency in Hz
void setFrequency(long lFreq_Hz) {
  // Phase-correct PWM: ICR1 = F_CPU / (2 * N * f)
  long lICR = 16000000L / (2L * lFreq_Hz);

  if(lICR < 20) {
    lICR = 20; // Floor -> very short period loses resolution
  }
  if(lICR > 65535) {
    lICR = 65535; // 16-bit ceiling
  }

  ICR1 = (unsigned int)lICR;
  OCR1A = ICR1 / 2; // Always 50% duty cycle
}

/// @brief Samples A0 repeatedly and returns the maximum voltage observed.
///   Peak detection is appropriate here because an AC waveform is being measured
///   The peak tracks the signal amplitude regardless of where in the cycle
///   each smaple lands.
/// @return Peak voltage in volts. Scaled using measured VCC from measureVCC()
float measureVPeak() {
  int iPeak = 0;
  for(int i = 0; i < iSamples; i++) {
    int iVal = doubleRead(A0);
    if(iVal > iPeak) iPeak = iVal;
    delayMicroseconds(8); // Space samples across cycles
  }
  return (iPeak / 1023.0f) * fV_ref;
}

/// @brief Sweeps frequency from lStart to lEnd in iSteps steps using
///   logarithmic spacing. Log spacing distributes steps evenly in
///   frequency ratio. Essential for resonance plots.
///   Checks for STOP command at every step.
/// @param lStart Start frequency in Hz.
/// @param lEnd End frequency in Hz
/// @param iSteps Number of frequency steps (max 500).
void runSweep(long lStart, long lEnd, int iSteps) {
  bSweepActive = true;
  bSweepStop = false;

  for(int i = 0; i < iSteps; i++) {
    // Check for mid-sweep STOP commnad
    if(Serial.available() > 0) {
      String strCmd = Serial.readStringUntil('\n');
      strCmd.trim();
      if(strCmd == "STOP") {
        bSweepStop = true;
      }
    }

    if(bSweepStop) break;

    // Logarithmic interpolation:
    // f_i = f_start * (f_end / f_start) ^ (i / (N - 1))
    float fT = (iSteps > 1) ? (float)i / (iSteps - 1) : 0.0f;
    long lFreq = (long)(lStart * powf((float)lEnd / lStart, fT));

    setFrequency(lFreq);
    delay(iSettle_ms);

    float fVpeak = measureVPeak();

    Serial.print("FREQ:");
    Serial.print(lFreq);
    Serial.print(",VPEAK:");
    Serial.println(fVpeak, 4);
  }

  // Restore 50 kHz default after sweep
  setFrequency(50000);

  Serial.println(bSweepStop ? "SWEEP:STOPPED" : "SWEEP:DONE");
  bSweepActive = false;
}

void setup() {
  Serial.begin(9600);
  setupTimer1();

  fV_ref = measureVCC();
  fV_pin = fV_ref * 0.97f; // GPIO HIGH typically 97% of VCC. Verify with multimeter or stick with 97%

  Serial.print("VCC:");
  Serial.println(fV_ref, 3);
  Serial.print("VPIN:");
  Serial.println(fV_pin, 3);
  Serial.println("LRC Resonance Detector ready");
}

void loop() {
  if(Serial.available() == 0) return;
  
  String strCmd = Serial.readStringUntil('\n');
  strCmd.trim();

  // SWEEP:start,end,steps -> begin frequency sweep
  if(strCmd.startsWith("SWEEP:") && !bSweepActive) {
    String strArgs = strCmd.substring(6);
    int iC1 = strArgs.indexOf(',');
    int iC2 = strArgs.lastIndexOf(',');

    if(iC1 > 0 && iC2 > iC1) {
      long lStart = strArgs.substring(0, iC1).toInt();
      long lEnd = strArgs.substring(iC1 + 1, iC2).toInt();
      int iSteps = strArgs.substring(iC2 + 1).toInt();

      if(lStart > 0 && lEnd > lStart && iSteps > 1 && iSteps <= 500) {
        runSweep(lStart, lEnd, iSteps);
      } else {
        Serial.println("ERROR:Invalid sweep parameters");
      }
    }
    return;
  }

  // STOP -> Abort active sweep
  if(strCmd == "STOP") {
    bSweepStop = true;
    return;
  }

  // SETPINV:x.xxx -> user-measured Pin 11 HIGH voltage
  if(strCmd.startsWith("SETPINV:")) {
    fV_pin = strCmd.substring(8).toFloat();
    Serial.print("VPIN:");
    Serial.println(fV_pin, 3);
    return;
  }

  // MEASURE_VCC -> re-measure supply
  if(strCmd == "MEASURE_VCC") {
    fV_ref = measureVCC();
    fV_pin = fV_ref * 0.97f; // Typically 97%. Check with multimeter
    Serial.print("VCC:");
    Serial.println(fV_ref, 3);
    return;
  }
}