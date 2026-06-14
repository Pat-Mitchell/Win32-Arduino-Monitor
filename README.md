# Win32 Arduino Monitor

A collection of Windows GUI applications that communicate with an Arduino Mega 2560 over serial, built from scratch using the Win32 API and a C++ OOP wrapper library.

---

## What This Is

This project was built as a structured learning exercise covering two domains simultaneously:

- **Win32 GUI programming** — event-driven architecture, GDI drawing, controls, serial I/O
- **Arduino/electronics** — analog sensing, digital I/O, RC circuits, hardware communication

Each project introduces new electrical theory and new Win32 concepts, building on the previous one.

---

## Wrapper Library

Rather than using raw Win32 calls throughout, a reusable OOP wrapper was built incrementally as each new need arose:

```
UIElement       — abstract base (HWND management, SetText, Enable/Disable)
├── Window      — message pump, WndProc trampoline via GWLP_USERDATA
├── Button      — BS_PUSHBUTTON wrapper
├── Label       — STATIC control wrapper
├── TextInput   — EDIT control (single-line and multiline/read-only)
└── ComboBox    — CBS_DROPDOWNLIST wrapper

SerialPort      — Win32 serial I/O (CreateFile, DCB, COMMTIMEOUTS)
Utils           — ScanComPorts, ParseFloat, ShowSaveDialog
```

---

## Projects

### Project 01: Ohm's Law Visualizer
Displays live V, I, R, and P values from a voltage divider circuit read via ADC.

**Theory:** Ohm's Law, voltage dividers, power dissipation  
**Hardware:** Potentiometer, fixed resistors  
**Win32:** Live numeric readouts updated on WM_TIMER

### Project 02: RC Circuit Time Constant Logger
Triggers charge and discharge cycles, plots the exponential voltage curve live, and calculates τ from the 63.2% crossover point.

**Theory:** Capacitor charge/discharge, τ = RC, diode discharge paths  
**Hardware:** 10kΩ resistor, 100µF electrolytic capacitor, 1N4148 diode  
**Win32:** GDI plot panel, CSV export via GetSaveFileName

### Project 03: PWM LED Dimmer with Frequency Display
Controls the brightness of an LED from Win32 app, plots the average voltage measured at the low-pass filter at a stable duty cycle and the settling voltage from a sudden increase in duty cycle.

**Theory:** PWM duty cycles, converting the raw digital signal to a smooth average using a low-pass filter, observing the effects of changing the τ = RC value of the low-pass filter (faster/slower response; larger/smaller ripples).  
**Hardware:** 220Ω, 1kΩ, 4.7kΩ, 5.6kΩ, 10kΩ, 1MΩ resistors; 10µF, 4.7µF electrolytic capacitors; Red LED.  
**WIN32:** Trackbar, Live numeric readouts, GDI plot panel, CSV export via GetSaveFileName  

### Project 04: Wheatstone Bridge with NTC Thermistor
Measures the change in resistance of a 10kΩ NTC Thermistor. Measured resistance is converted into a temperature reading with a line plot showing the change over time. Includes a stage where the Arduino voltage is calibrated to improve the accuracy of readings.
**Theory:** Wheatstone bridges. What happens in a theoretical balanced bridge and what occurs when it is unbalanced. Converting raw digital signal to a temperature reading using the beta formula.  
**Hardware:** 10kΩ resistors, 10kΩ ntc thermistor.  
**Win32:** Live numeric readouts, GDI plot panel, CSV export.  

### Project 05: AC Phasor Power Triangle
Display the theoretical result of $V_{rms}$, frequency, capacitance, inductance, and resistance in an RC circuit and display the result with numeric readouts and a graphical display of the resulting phasor diagram and power triangle.  
**Theory:** Understanding what AC current is. $V_{RMS}$ is used instead of $V_{peak}$ to describe the equivalent voltage in a DC circuit. Reactance and its relation to resistance in a DC circuit. The effects of capacitors and inductors making a circuit more capacitive/inductive, shifting the phase of the voltage/current, and how engineers aim for resonance to maximize the power factor.  
**Hardware:** None. This project was based in theory.  
**Win32:** Live numeric readouts that update live with changes in inputs. Two GDI panels that display the resulting phasor diagram and power triangle of the user's inputs.  

---

## Build

All projects compile with g++:

```bash
g++ projects\Ohm's Law Project\OhmsLaw.cpp wrappers\win32Wrappers\Window.cpp wrappers\serialPortWrappers\SerialPort.cpp Utils\Utils.cpp -o OhmsLaw -mwindows -DUNICODE -D_UNICODE
```

Precompiled headers are supported via `pch.h` and `build_pch.bat` for faster rebuild times.

**Hardware:** Arduino Mega 2560 on COM3 (selectable via dropdown in each app)

---

## Lessons

The `/lessons` folder contains the raw Win32 code written before the wrapper existed. Comparing `/lessons/04_serial_raw` against `/projects/01_ohms_law` shows concretely what the wrapper abstraction replaced.

---

## Planned Projects

- LRC Resonance Detector
- H-Bridge DC Motor Controller
- Op-Amp Signal Amplifier / Oscilloscope
- I2C Sensor Dashboard
- PID Motor Speed Controller
