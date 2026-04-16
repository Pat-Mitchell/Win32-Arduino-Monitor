/// @file hello_arduino.ino
/// @brief Sanity check — Serial print + LED blink hello world.

// The built-in LED pin is a named constant on all standard Arduino boards.
// Uno/Nano = pin 13, Leonardo = pin 13, etc.
const int iPinLed = LED_BUILTIN;

/// @function setup
/// @brief Runs once on power-on or reset. Used for initialization.
/// @return void
void setup()
{
    // 9600 baud — match this value in the Serial Monitor dropdown.
    Serial.begin(9600);

    pinMode(iPinLed, OUTPUT);

    Serial.println("Hello, Arduino! Preflight check OK.");
}

/// @function loop
/// @brief Runs repeatedly after setup(). The main execution loop.
/// @return void
void loop()
{
    digitalWrite(iPinLed, HIGH);   // LED on
    delay(500);                    // Wait 500ms
    digitalWrite(iPinLed, LOW);    // LED off
    delay(500);

    Serial.println("Blink.");
}