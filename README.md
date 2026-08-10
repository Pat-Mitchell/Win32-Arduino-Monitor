# Win32 Arduino Monitor

A collection of Windows GUI applications that communicate with an Arduino Mega 2560 over serial, built from scratch using the Win32 API and a C++ OOP wrapper library.

---

## What This Is

This project was built as a structured learning exercise covering two domains simultaneously:

- **Win32 GUI programming** - event-driven architecture, GDI drawing, controls, serial I/O
- **Arduino/electronics** - analog sensing, digital I/O, RC circuits, hardware communication

Each project introduces new electrical theory and new Win32 concepts, building on the previous one.

---

## Wrapper Library

Rather than using raw Win32 calls throughout, a reusable OOP wrapper was built incrementally as each new need arose:

```
UIElement       - abstract base (HWND management, SetText, Enable/Disable)
├── Window      - message pump, WndProc trampoline via GWLP_USERDATA
├── Button      - BS_PUSHBUTTON wrapper
├── Label       - STATIC control wrapper
├── TextInput   - EDIT control (single-line and multiline/read-only)
├── ComboBox    - CBS_DROPDOWNLIST wrapper with AddItem/SelectFirst/GetSelected
├── Trackbar    - msctls_trackbar32 wrapper with TBM_SETRANGE/TBM_GETPOS
└── ProgressBar - PROGRESS_CLASS wrapper with SetRange/SetPos/StepIt

SerialPort      - Win32 serial I/O (CreateFile, DCB, COMMTIMEOUTS)
XInputController- XInput gamepad wrapper with radial deadzone, polling, and per-axis rescaling
PlotPanel       - Head-only reusable GDI plot
Utils           - ScanComPorts, ParseFloat, ShowSaveDialog, FormatReadout, MapFloat, ClampFloat, DrawArrow, DrawArcArrow
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

### Project 06: LRC Resonance Detector
Sweeps PWM frequency via Timer 1 register control across a series LRC circuit. Measures peak voltage across a sense resistor at each step and plots the frequency response curve. Resonant peak detection and comparison against theoretical $f_0$.  
**Theory:** Inductive/capacitive reactance, resonant frequency ($f_0={1 \over 2\pi\sqrt{LC}}$), Q factor, voltage magnification at resonance, Arduino Timer 1 register control  
**Hardware:**100µH inductor, 100nF capacitor, 220Ω sense resistor  
**Win32:**Frequency response plot (log-spaced X axis), progress bar during sweep, $f_0$ measured vs theoretical comparison, CSV export  

### Project 07: H-Bridge DC Motor Controller
Bidrectional variable-speed DC motor control via an L298N H-bridge module. Win32 slider spans -100% to +100% with a $\pm 5\%$ dead zone. Dead time enforced on direction reversal. Stall detection cuts power after sustained overcurrent.
**Theory:**H-bridge topology, shoot-through and dead time, back-EMF, flyback diodes, inductive kickback, PWM motor speed control, current sensing
**Hardware:** L298N H-bridge module, 3–6V DC motor, $1\Omega$ sense resistor
**Win32:** Bidirectional Trackbar, scrolling current history plot, Brake/Coast/Clear Stall controls, CSV export

### Project 08: 3-Phase Power Visualizer
Animates a three-phase AC system in both star and delta configurations with selectable ABC/ACB phase sequence.
**Theory:** Three-phase power, star vs delta topology, line vs phase voltage, phase sequence, rotating phasor systems
**Win32:** Three-channel cosine waveform panel (360-segment polylines), phasor panel with phase phasors and tip-to-tip line voltage vectors (V_AB/V_BC/V_CA)

### Project 09: AC Induction Motor Visualizer
Animates the rotating magnetic field and torque-speed curve of a three-phase induction motor across the full slip range.
**Theory:** Rotating magnetic field, slip, synchronous vs rotor speed, torque-speed curve (starting torque, breakdown torque, full-load point)
**Win32:** Animated rotating field panel (three winding arrows + net field resultant), torque-speed curve panel with key point markers, 16ms animation timer

---

## Build

All projects compile with g++:

```bash
g++ HBridgeMotor.cpp ^
  ..\..\..\wrappers\win32Wrappers\Window.cpp ^
  ..\..\..\wrappers\serialPortWrappers\SerialPort.cpp ^
  ..\..\..\Utils\Utils.cpp ^
  -o HBridgeMotor.exe ^
  -mwindows -DUNICODE -D_UNICODE -lcomctl32
```

Add -lxinput for any project using XInputController.
Add -lcomctl32 for any project using Trackbar or ProgressBar.

Precompiled headers are supported via pch.h and build_pch.bat for faster rebuild times. Each project folder contains its own build.bat with the correct paths and flags.

**Hardware:** Arduino Mega 2560 on COM3 (selectable via dropdown in each app)

---

## Lessons

The `/lessons` folder contains the raw Win32 code written before the wrapper existed. Comparing `/lessons/04_serial_raw` against `/projects/01_ohms_law` shows concretely what the wrapper abstraction replaced.

---

## Wiring Diagrams

SVG wiring diagrams for all hardware projects are in /docs/diagrams/. GitHub renders SVG natively — diagrams display inline when browsing the repo.

---

## Planned Projects

- Op-Amp Signal Amplifier / Oscilloscope
- I2C Sensor Dashboard
- PID Motor Speed Controller
- Bode Plot Generator
- Diode IV Curve
