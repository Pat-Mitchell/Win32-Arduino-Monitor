/// @file DualMotorController.ino
/// @brief Receives dual motor speed commands from the Win32 app 
///   over serial and drives two DC motors independently via a single
///   L298N H-Bridge.
///
/// Serial frame format from Win32
///   "M1:xxx,M2:xxx\n"
///   Speed is in [-255, +255]
///
/// Wiring (L298N)
///   Motor 1: IN1 -> pin7, IN2 -> pin 8, ENA -> pin 9
///   Motor 2: IN3 -> pin 10, IN4 -> pin 11, ENB -> pin 12
///   Current sense: not wired

// ────── ⋆⋅☆⋅⋆ ────────
//   Pin assignments
// ────── ⋆⋅☆⋅⋆ ────────
const int PIN_M1_IN1 = 7;
const int PIN_M1_IN2 = 8;
const int PIN_M1_ENA = 9;

const int PIN_M2_IN3 = 10;
const int PIN_M2_IN4 = 11;
const int PIN_M2_ENB = 12;

// ────── ⋆⋅☆⋅⋆ ────────
//   Timing constants
// ────── ⋆⋅☆⋅⋆ ────────
const unsigned long DEAD_TIME_MS = 100; // Direction change settling time

// ────── ⋆⋅☆⋅⋆ ────────
//     Motor state
// ────── ⋆⋅☆⋅⋆ ────────
struct MotorState {
  int iCurrentSpeed;
  int iTargetSpeed;
  bool bInDeadTime; // TRUE during direction change settling
  unsigned long ulDeadTimerStart; // millis() when dead time began
};

MotorState motor1 = { 0, 0, false, 0 };
MotorState motor2 = { 0, 0, false, 0 };

// ────── ⋆⋅☆⋅⋆ ────────
// Serial line buffer
// ────── ⋆⋅☆⋅⋆ ────────
static char s_arrLineBuf[64];
static int s_iLineLen = 0;

// ────── ⋆⋅☆⋅⋆ ────────
// function prototypes
// ────── ⋆⋅☆⋅⋆ ────────
void applyMotor(int iPin_IN_A, int iPin_IN_B, int iPin_EN, int iSpeed);
void updateMotor(MotorState& motor, int iPin_IN_A, int iPin_IN_B, int iPin_EN);
void parseFrame(const char* szFrame);
int parseIntField(const char* szSrc, const char* szKey);

// ────── ⋆⋅☆⋅⋆ ────────
//        Setup
// ────── ⋆⋅☆⋅⋆ ────────
void setup() {
  pinMode(PIN_M1_IN1, OUTPUT);
  pinMode(PIN_M1_IN2, OUTPUT);
  pinMode(PIN_M1_ENA, OUTPUT);

  pinMode(PIN_M2_IN3, OUTPUT);
  pinMode(PIN_M2_IN4, OUTPUT);
  pinMode(PIN_M2_ENB, OUTPUT);

  // initial safe state -> both motors stopped
  applyMotor(PIN_M1_IN1, PIN_M1_IN2, PIN_M1_ENA, 0);
  applyMotor(PIN_M2_IN3, PIN_M2_IN4, PIN_M2_ENB, 0);

  Serial.begin(9600);
}

// ────── ⋆⋅☆⋅⋆ ────────
// Loop
// ────── ⋆⋅☆⋅⋆ ────────
void loop() {
  // Serial read
  while(Serial.available() > 0) {
    char c = (char)Serial.read();

    if(c == '\n') {
      s_arrLineBuf[s_iLineLen] = '\0';
      parseFrame(s_arrLineBuf);
      s_iLineLen = 0;
    } else if (s_iLineLen < (int)(sizeof(s_arrLineBuf) - 1)) {
      s_arrLineBuf[s_iLineLen++] = c;
    }
  }

  // Update motors
  updateMotor(motor1, PIN_M1_IN1, PIN_M1_IN2, PIN_M1_ENA);
  updateMotor(motor2, PIN_M2_IN3, PIN_M2_IN4, PIN_M2_ENB);
}

/// @brief Set the direction pins and PWM for one motor
/// @param iPIN_IN_A Motor pin 1 [HIGH, LOW]
/// @param iPIN_IN_B Motor pin 2 [HIGH, LOW]
/// @param iPin_EN Enable pin
/// @param iSpeed PWM speed
void applyMotor(int iPin_IN_A, int iPin_IN_B, int iPin_EN, int iSpeed) {
  if(iSpeed == 0) {
    digitalWrite(iPin_IN_A, LOW);
    digitalWrite(iPin_IN_B, LOW);
    analogWrite(iPin_EN, 0);
  } else if(iSpeed > 0) {
    digitalWrite(iPin_IN_A, HIGH);
    digitalWrite(iPin_IN_B, LOW);
    analogWrite(iPin_EN, iSpeed);
  } else {
    digitalWrite(iPin_IN_A, LOW);
    digitalWrite(iPin_IN_B, HIGH);
    analogWrite(iPin_EN, -iSpeed);
  }
}

/// @brief Apply speed changes to motor
///   Protection against shoot through. Set deadtime to TRUE on direction change, FALSE + apply new speed after elapsed time
void updateMotor(MotorState& motor, int iPin_IN_A, int iPin_IN_B, int iPin_EN) {
  bool bDirectionChange = (motor.iCurrentSpeed > 0 && motor.iTargetSpeed < 0) || (motor.iCurrentSpeed < 0 && motor.iTargetSpeed > 0);

  // Begin dead time: stop motor to prevent shoot through
  if(bDirectionChange && !motor.bInDeadTime) {
    applyMotor(iPin_IN_A, iPin_IN_B, iPin_EN, 0);
    motor.bInDeadTime = true;
    motor.ulDeadTimerStart = millis();
    return;
  }

  if(motor.bInDeadTime) {
    if(millis() - motor.ulDeadTimerStart >= DEAD_TIME_MS) {
      // Dead time elapsed. Safe to apply new direction
      motor.bInDeadTime = false;
    } else {
      // Still in dead time
      return;
    }
  }

  // Apply target speed if it differs from current
  if(motor.iTargetSpeed != motor.iCurrentSpeed) {
    applyMotor(iPin_IN_A, iPin_IN_B, iPin_EN, motor.iTargetSpeed);
    motor.iCurrentSpeed = motor.iTargetSpeed;
  }
}

/// @brief Parse messages from win32 app and guard against invalid PWM values
void parseFrame(const char* szFrame) {
  int iSpeed1 = parseIntField(szFrame, "M1:");
  int iSpeed2 = parseIntField(szFrame, "M2:");

  // Clamp to valid PWM range
  if(iSpeed1 < -255) iSpeed1 = -255;
  if(iSpeed1 > 255) iSpeed1 = 255;
  if(iSpeed2 < -255) iSpeed2 = -255;
  if(iSpeed2 > 255) iSpeed2 = 255;

  motor1.iTargetSpeed = iSpeed1;
  motor2.iTargetSpeed = iSpeed2;
}

/// @brief Seraches szSrc for szKey and returns the interger values immediately following it.
///   returns 0 if the key is not found.
int parseIntField(const char* szSrc, const char* szKey) {
  const char* pFound = strstr(szSrc, szKey);
  if(!pFound) {
    return 0;
  }

  pFound += strlen(szKey);
  return (int)atoi(pFound);
}