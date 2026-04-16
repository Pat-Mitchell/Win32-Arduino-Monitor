/// @file serial_hello.ino
/// @brief Broadcasts a counter over Serial every second.
///        Baud rate must match the Win32 app (9600)

int iCounter = 0;
bool bLedOn = false;

/// @function setup
/// @description Initializes Serial at 9600 baud
///              and LED pin
/// @return void

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Arduino ready!");
}

/// @function loop
/// @description Sends an incrementing counter once per second
/// @return void
void loop() {
  // ────── ⋆⋅☆⋅⋆ ────────
  // Check for incoming commands before doing anything else.
  // Serial.available() returns the number of bytes waiting in the
  // receive buffer. We only read when something is actually there.
  // ────── ⋆⋅☆⋅⋆ ────────
  if(Serial.available() > 0) {
    String strCmd = Serial.readStringUntil('\n');
    strCmd.trim(); // Strip whitespace and \r so comparisons work reliably

    if(strCmd == "LED_ON") {
      bLedOn = true;
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("[LED on ]");
    } else if(strCmd == "LED_OFF") {
      bLedOn = false;
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("[LED off]");
    }
  }

  Serial.print("Count: ");
  Serial.println(iCounter);
  iCounter++;
  delay(1000);
}
