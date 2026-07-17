/// @file hbridge_motor.ino
/// @brief H-Bridge DC Motor Controller.
///    Controls an L298N H-bridge bidirectionally via PWM on pin 9.
///    Reads current draw via a 1 Ohm sense resistor in parallel
///    with a low-pass filter on A0.

// ────── ⋆⋅☆⋅⋆ ────────
// Pin assignments
// ────── ⋆⋅☆⋅⋆ ────────
const int iPinENA = 9; // PWM speed control -> L298 ENA (Timer 2 on Mega)
const int iPinIN1 = 7; // HIGH/LOW Direction -> L298 IN1
const int iPinIN2 = 8; // HIGH/LOW Direction -> L298 IN2
const int iPinSense = A0; // Voltage across R_sense

// ────── ⋆⋅☆⋅⋆ ────────
// Component values
// ────── ⋆⋅☆⋅⋆ ────────
const float fR_sense = 1.0; // Sense resistor in Ohms
const float fStall_A = 1.2; // Stall current threshold (Amps)
const int iStall_ms = 500; // Sustained overcurrent duration = stall (ms)
const int iDeadTime_ms = 100; // Direction change dead time (ms)
const int iReport_ms = 100; // Telemetry interval (ms)

// ────── ⋆⋅☆⋅⋆ ────────
// Runtime state
// ────── ⋆⋅☆⋅⋆ ────────
float fV_ref = 5.0;
float fV_pin = 4.85; // actual ENA pin HIGH voltage

int iSpeed = 0; // Current speed: -100 to 100
bool bBraking = false;
bool bCoast = true;
bool bStall = false;
long lStallStart = 0;
int iLastDirection = 0; // -1 reverse, 0 stop, +1 forward

/// @brief Back-calculates VCC via ATmega internal bandgap reference.
///   Saves and restores ADCSRA and restores ADMUX to A0 on exit.
/// @return VCC in volts
float measureVCC() {
  uint8_t prevADCSRA = ADCSRA;

  // Switch to bandgap reference on internal channel
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // /128 for stable reading
  delay(5);

  // Throwaway conversion. Settles the bandgap reference
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
  ADCSRA = prevADCSRA;

  return 1125300.0 / lRaw / 1000.0;
}

/// @brief Reads analog pin twice, discards first result.
///   Settles the internal ADC sample-and-hold capacitor
/// @param iPin Analog pin number
/// @return Settled ADC reading (0-1023)
int doubleRead(int iPin) {
  analogRead(iPin);
  return analogRead(iPin);
}

/// @brief Sets motor speed and direction through the l298N.
///    Enforces dead time when direction actually reverses.
/// @param iTargetSpeed Speed -100 to +100. 0 = coast
void setMotor(int iTargetSpeed) {
  iTargetSpeed = constrain(iTargetSpeed, -100, 100);

  int iTargetDir = (iTargetSpeed > 0) ? 1
                 : (iTargetSpeed < 0) ? -1
                 : 0;
  
  // Dead time: only needed when motor is actively dirven in the opposite direction
  if(iLastDirection != 0 && iTargetDir != 0 && iTargetDir != iLastDirection) {
    analogWrite(iPinENA, 0);
    digitalWrite(iPinIN1, LOW);
    digitalWrite(iPinIN2, LOW);
    delay(iDeadTime_ms);
  }

  bBraking = false;
  bCoast = (iTargetSpeed == 0);
  iSpeed = iTargetSpeed;

  int iPwmVal = (int)(abs(iTargetSpeed) / 100.0 * 255.0);

  if(iTargetSpeed > 0) {
    digitalWrite(iPinIN1, HIGH);
    digitalWrite(iPinIN2, LOW);
    analogWrite(iPinENA, iPwmVal);
    iLastDirection = 1;
  } else if(iTargetSpeed < 0) {
    digitalWrite(iPinIN1, LOW);
    digitalWrite(iPinIN2, HIGH);
    analogWrite(iPinENA, iPwmVal);
    iLastDirection = -1;
  } else {
    analogWrite(iPinENA, 0);
    digitalWrite(iPinIN1, LOW);
    digitalWrite(iPinIN2, LOW);
    iLastDirection = 0;
  }
}

/// @brief Applies active braking (IN1 = IN2 = HIGH, ENA full on)
///   Motor windings short-circuit through the bridge
///   Faster stop than coast. Generates a brief current spike.
void setBrake() {
  analogWrite(iPinENA, 255);
  digitalWrite(iPinIN1, HIGH);
  digitalWrite(iPinIN2, HIGH);
  bBraking = true;
  bCoast = false;
  iSpeed = 0;
  iLastDirection = 0;
}

/// @brief Reads the sense resistor voltage on A0 and derives motor current.
///    Uses double read and measured VCC.
/// @return Motor current in Amps.
float readCurrentA() {
  float fV_sense = (doubleRead(iPinSense) / 1023.0f) * fV_ref;
  return fV_sense / fR_sense;
}

/// @brief Monitors current and cuts power if stall persists beyond threshold.
///   Resets stall timer when current drops back to normal.
/// @param fCurrent Current reading in Amps.
void checkStall(float fCurrent) {
  if(fCurrent >= fStall_A && !bBraking && !bCoast && iSpeed != 0) {
    if(lStallStart == 0) lStallStart = millis();

    if((millis() - lStallStart) >- (long)iStall_ms) {
      setMotor(0);
      bStall = true;
      Serial.println("ALERT:STALL");
    }
  } else {
    lStallStart = 0;
    if(bStall && fCurrent < fStall_A * 0.8f) bStall = false;
  }  
}

void setup() {
  Serial.begin(9600);

  pinMode(iPinENA, OUTPUT);
  pinMode(iPinIN1, OUTPUT);
  pinMode(iPinIN2, OUTPUT);

  // Start in coast state
  analogWrite(iPinENA, 0);
  digitalWrite(iPinIN1, LOW);
  digitalWrite(iPinIN2, LOW);

  // ADC at /16 prescaler for faster current sense reads
  ADCSRA = (ADCSRA & ~0x07) | _BV(ADPS2);

  fV_ref = measureVCC();
  fV_pin = fV_ref * 0.97f; // GPIO HIGH typically 97% of VCC

  Serial.print("VCC:");
  Serial.println(fV_ref, 3);
  Serial.print("VPIN:"); 
  Serial.println(fV_pin, 3);
  Serial.println("H-Bridge Motor Controller ready.");
}

void loop() {
  // ────── ⋆⋅☆⋅⋆ ────────
  // Command Handling
  // ────── ⋆⋅☆⋅⋆ ────────
  if(Serial.available() > 0) {
    String strCmd = Serial.readStringUntil('\n');
    strCmd.trim();

    if(strCmd.startsWith("SPEED:")) {
      if(!bStall) {
        int iVal = strCmd.substring(6).toInt();
        setMotor(iVal);
      } else {
        // Reject speed commands while stalled
        Serial.println("ALERT:STALL");
      }
    } else if(strCmd == "BRAKE") {
      bStall = false;
      setBrake();
    } else if(strCmd == "COAST") {
      bStall = false;
      setMotor(0);
    } else if(strCmd == "CLEARSTALL") {
      bStall = false;
      lStallStart = 0;
      setMotor(0);
      Serial.println("STALL:CLEARED");
    } else if(strCmd == "MEASURE_VCC") {
      fV_ref = measureVCC();
      fV_pin = fV_ref * 0.97f;
      Serial.print("VCC:");
      Serial.println(fV_ref, 3);
    } else if(strCmd.startsWith("SETVPIN:")) {
      fV_pin = strCmd.substring(8).toFloat();
      Serial.print("VPIN:");
      Serial.println(fV_pin, 3);
    }
  }

  // ────── ⋆⋅☆⋅⋆ ────────
  // Periodic Telemetry
  // ────── ⋆⋅☆⋅⋆ ────────
  static long lLastReport = 0;
  long lNow = millis();
  if(lNow - lLastReport < iReport_ms) return;
  lLastReport = lNow;

  float fCurrent = readCurrentA();
  checkStall(fCurrent);

  // Direction string
  const char* szDir;
  if(bBraking) szDir = "B";
  else if(bCoast || iSpeed  == 0) szDir = "C";
  else if(iSpeed > 0) szDir = "F";
  else szDir = "R";

  // Format: "SPD:xx,DIR:F,CURR:x.xxx,VCC:x.xxx,STALL:n/n"
  Serial.print("SPD:");
  Serial.print(abs(iSpeed));
  Serial.print(",DIR:");
  Serial.print(szDir);
  Serial.print(",CURR:");
  Serial.print(fCurrent, 3);
  Serial.print(",VCC:");
  Serial.print(fV_ref, 3);
  Serial.print(",STALL:");
  Serial.println(bStall ? 1 : 0);
}