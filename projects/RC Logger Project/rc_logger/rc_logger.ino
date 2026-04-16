/// @file rc_logger.ino
/// @brief RC Circuit Time Constant Logger.
///        Listens for CHARGE and DISCHARGE commands from the Win32 app,
///        drives D2/D3 accordingly, and streams timestamped voltage
///        samples over Serial at 9600 baud.

const float fR = 10000.0; // 10k ohm resistor
const float fC = 0.0001; // 100 uF Capacitor
const float fV_ref = 5.0; // 5V ADC reference voltage
const float fAdc_max = 1023; // ADC max reading (10-bit)
const int iPinCharge = 2; // Charge trigger pin - HIGH to charge
const int iPinDischarge = 3; // Discharge trigger pin - HIGH to reverse-bias diode during charge
const float fDelta_threshold = 0.005; // Coltage change threshold. Below which cycle is considered complete
const int iStable_count = 10; // Number of consecutive stable readings before declaring cycle done
const int iSample_ms = 50; // Milliseconds between ADC samples during ative cycle

// ─────── ⋆⋅☆⋅⋆ ────────
//         State
// ─────── ⋆⋅☆⋅⋆ ────────
enum State { IDLE, CHARGING, DISCHARGING };
State state_current = IDLE;

float fV_prev = 0.0;
int iStable_ticks = 0;
long lCycle_start = 0;

void setup() {
  Serial.begin(9600);
  pinMode(iPinCharge, OUTPUT);
  pinMode(iPinDischarge, OUTPUT);

  // Start idle; both pins LOW, both paths open, not charging capacitor
  digitalWrite(iPinCharge, LOW);
  digitalWrite(iPinDischarge, LOW);

  Serial.println("RC Logger ready");
}

/// @returns Voltage in volts
/// @brief Converts a raw analogRead on A0 to a float voltage
float readVoltage() {
  int iRaw = analogRead(A0);
  return (iRaw / fAdc_max) * fV_ref;
}

/// @param lMs Elapsed milliseconds
/// @param fV Voltage Reading
/// @returns void
/// @brief Transmits a timestamped voltage sample.
///        Format: "T:xxxx,V:x.xxx\n"
///        T is milliseconds elapsed since cycle start
void sendSample(long lMs, float fV) {
  Serial.print("T:"); Serial.print(lMs);
  Serial.print(",V:"); Serial.println(fV, 3);
}

/// @param fTau_measured Measured tau derived from the 63.2% crossover
/// @returns void
/// @brief Notifies the Win32 app that the cycle has stabilised
void sendCycleDone(float fTau_measured) {
  Serial.print("DONE,TAU:"); Serial.println(fTau_measured, 4);
}

/// @returns void
/// @brief Reads incoming Serial bytes and acts on CHARGE or DISCHARGE
void checkCommand() {
  if(Serial.available() == 0) return;

  String strCmd = Serial.readStringUntil('\n');
  strCmd.trim();

  if(strCmd == "CHARGE" && state_current == IDLE) {
    // Both HIGH. Charge path open, diode reverse-biased
    digitalWrite(iPinCharge, HIGH);
    digitalWrite(iPinDischarge, HIGH);

    state_current = CHARGING;
    lCycle_start = millis();
    fV_prev = readVoltage();
    iStable_ticks = 0;

    Serial.println("CYCLE:CHARGE");
  } else if(strCmd == "DISCHARGE") {
    // Discharge through the resistor
    digitalWrite(iPinCharge, LOW);
    digitalWrite(iPinDischarge, HIGH);

    state_current = DISCHARGING;
    lCycle_start = millis();
    fV_prev = readVoltage();
    iStable_ticks = 0;

    Serial.println("CYCLE:DISCHARGE");
  } else if(strCmd == "RESET") {
    digitalWrite(iPinCharge, LOW);
    digitalWrite(iPinDischarge, LOW);
    state_current = IDLE;
    Serial.println("CYCLE:IDLE:");
  }
}

// ─────── ⋆⋅☆⋅⋆ ────────
//     Tau Estimation
// ─────── ⋆⋅☆⋅⋆ ────────
// During charging, tau is the time at which V crosses 63.2% of V_ref.
// During discharging, t is the time at which V drops below 36.8% of V_start.
// We track these crossover points as samples arrive.

float fV_cycle_start = 0.0; // Voltage at the moment the cycle began
float fTau_crossover = -1.0; // Elapsed ms when 63.2% threshold was crossed

/// @param fV Current Voltage
/// @param lMs Elapsed milliseconds
/// @brief Checks whether the current voltage has crossed the tau threshold
///        and records the elapsed time if so.
void checkTauCrossover(float fV, long lMs) {
  if(fTau_crossover >= 0.0) return; // Already found

  if(state_current == CHARGING) {
    // tau crossover at 63.2% of V_ref
    float fTarget = fV_ref * 0.632;
    if(fV >= fTarget) fTau_crossover = lMs;
  } else if(state_current == DISCHARGING) {
    // tau crossover at 36.8% of V_cycle_start
    float fTarget = fV_cycle_start * 0.368;
    if(fV <= fTarget) fTau_crossover = lMs;
  }
}

/// @brief Checks for commands and, during an active cycle, samples A0
///        every iSample_ms milliseconds. Detects cycle completeion by 
///        watching for voltage rate-of-change dropping below threshold
void loop() {
  checkCommand();

  if(state_current == IDLE) return;

  static long lLast_sample = 0;
  long lNow = millis();

  if(lNow - lLast_sample < iSample_ms) return;
  lLast_sample = lNow;

  long lElapsed = lNow - lCycle_start;
  float fV = readVoltage();

  // Record V at cycle start (first sample)
  if(lElapsed < iSample_ms) fV_cycle_start = fV;

  sendSample(lElapsed, fV);
  checkTauCrossover(fV, lElapsed);

  // ─────── ⋆⋅☆⋅⋆ ────────
  //  Stability detection
  // ─────── ⋆⋅☆⋅⋆ ────────
  // Declare cycle done when voltage stops changing.
  // Uses absolute Delta between this sample and the previous one.
  float fDelta = abs(fV - fV_prev);
  fV_prev = fV;

  if(fDelta < fDelta_threshold) iStable_ticks++;
  else iStable_ticks = 0;

  if(iStable_ticks >= iStable_count) {
    // convert crossover ms to seconds to tau
    float fTau = (fTau_crossover > 0.0) ? (fTau_crossover / 1000.0) : -1.0;

    sendCycleDone(fTau);

    // Return to idle after discharge
    if(state_current == DISCHARGING) {
      digitalWrite(iPinCharge, LOW);
      digitalWrite(iPinDischarge, LOW);
      state_current = IDLE;
      fTau_crossover = -1.0;
    } else {      
      state_current = IDLE;
      fTau_crossover = -1.0;
    }
  }
}



















