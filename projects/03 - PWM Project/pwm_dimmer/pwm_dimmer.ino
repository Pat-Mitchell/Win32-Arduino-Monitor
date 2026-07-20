/// @file pwm_dimmer.ino
/// @brief PWM LED Dimmer with frequency display.
///   Receives duty cycle commands from the Win32 app, drives an
///   LED via PWM on pin 9, and streams duty cycle + filtered average
///   voltage back over Serial every 200ms.

const int iPinPWM = 9; // PWM output pin
const float fV_ref = 5.0; // ADC reference voltage
const float fAdc_max = 1023.0; // ADC max reading (10-bit)
int iDuty = 0; // Current duty cycle (0-100). Updated on each valid command.
const int iReport_ms = 200; // Interval between ADC readback transmissions in ms

// ==================================

void setup() {
  Serial.begin(9600);
  pinMode(iPinPWM, OUTPUT);
  analogWrite(iPinPWM, 0);
  Serial.println("PWM Dimmer ready!");
}

/// @function checkCommand
/// @brief Reads incoming Serial bytes and parses "PWM:xx\n" commands.
///   Updates iDuty and calls analogWrite immediately on receipt.
///   Clamps received value to 0-100 defensively
/// @returns void
void checkCommand() { 
  if(Serial.available() == 0) return;

  String strCmd = Serial.readStringUntil('\n');
  strCmd.trim();

  // Expected format: "PWM:xx" where xx = 0-100
  if(strCmd.startsWith("PWM:")) {
    String strVal = strCmd.substring(4); // Strips "PWM:"
    int iReceived = strVal.toInt();

    // Clamp to valid range
    iDuty = constrain(iReceived, 0, 100);

    // Convert 0-100% to analogWrite 0-255
    int iPwmVal = (int)((iDuty / 100.0) * 255.0);
    analogWrite(iPinPWM, iPwmVal);
  } else if (strCmd == "RESET"){
    iDuty = 0;
    analogWrite(iPinPWM, 0);
    Serial.println("PWM:RESET");
  }
}

/// @function reportReadback
/// @brief Samples A0 and transmits duty cycle and filtered average
///   voltage. Format: "DUTY:xx,VAVG:x.xxx\n"
///   Called on a timer rather than every loop tick to allow the RC filter
///   to settle between duty cycle changes.
/// @returns void
void reportReadback() {
  static long lLastReport = 0;
  long lNow = millis();

  if(lNow - lLastReport < iReport_ms) return;
  lLastReport = lNow;

  int iRaw = analogRead(A0);
  float fVavg = (iRaw / fAdc_max) * fV_ref;

  Serial.print("DUTY:"); Serial.print(iDuty);
  Serial.print(",VAVG:"); Serial.println(fVavg, 3);
}

/// @brief Main loop
void loop() {
  checkCommand();
  reportReadback();
}
