# Arduino Timer Registers

The `analogWrite()` and `pinMode()` commands cannot natively change the fequency of a PWM signal. THey are designed foremost to only control the duty cycle. The frequency is initially fixed to 490Hz for most PWM pins and 980Hz for a few select others.

To change the fequency, the Timer Control Registers need to be modified.The trade-off of changing the timer's prescaler is that functions that rely on the same timers (such as `millis()`, `delay()`, and `tone()`) will also be effected.

### The Prescaler

The timer prescaler and Input Capture Register (ICR) are hardware components used to change both the overall speed and the maximum count length of the timer, which directly sets the exact PWM frequency and resolution. Timer 1 is a 16-bit timer unlike timers 0 and 2 that are 8-bit.

The prescaler acts as a clock frequency divider. The base system clock for the microcontroller is typically 16 MHz and is too fast for many practical applications. The prescaler intercepts the clock signal and divides it by a fixed factor before reaching Timer 1. Supported values are 1, 8, 64, 256, and 1024. The formula is simply:

$${\text {Base system clock frequency} \over \text {Prescale factor}} = \text {Timer counter rate}$$

### The Input Capture Register (ICR)

The ICR is designed the capture the exact timestamp of an external incoming signal. In PWM generation, it defines the top value of the timer counter. For an 8-bit timer, the timer counts up from 0 to 255 (its top value) before resetting. Modifying the ICR allows for precise control of PWM frequencies that are impossible to hit by just using the prescaler.

### The Custom PWM Frequency Formula

$$f_{PWM} = {f_{\text {clk}} \over 2 \times N \times \text {ICR}}$$

Where:

- $f_{\text PWM}$ is the resulting PWM frequency in Hz.

- $f_{\text {clk}}$ is the system clock speed in Hz (16,000,000 Hz).

- $N$ is the prescaler value (1, 8, 64, 256, 1024).

- $\text {ICR}$ is the value loaded into the Input Capture Register (0 - 65,535).

**Example**:

The target frequency is ~50kHz at prescaler 1:

$\text {ICR} = {f_{\text {clk}} \over 2 \times N \times f_{\text {target}}}$  
$\text {ICR} = {\text {16,000,000} \over (2 \times 1 \times \text {50,000})}$  
$\text {ICR} = 160$

### How Duty Cycle is set Alongside Frequency

`OCR1A` set the compare match point that determines whn the pin switches:  
$\text {Duty cycle} = {\text {OCR1A} \over \text {ICR}}$  
For a 50% duty cycle: $\text {OCR1A} = {\text {ICR} \over 2}$.  
Every time the ICR is changed to change frequency, the `OCR1A` is also updated to maintain the same duty cycle.

### Fast PWM and Phase Correct PWM

Fast PWM and Phase Correct PWM are two hardware methods the arduino uses to count and generate a PWM signal.

**Fast PWM (Single-Slope Counting)**

In Fast PWM, the timer counts up only from 0 to its max value (Top), and then instantly resets to 0. The pin turns HIGH when the timer resets to 0 and LOW when the timer hits the designated duty cycle value. Because the timer only travels in one direction (0 -> TOP -> clear), it completes a full cycle twice as fast as Phase Correct PWM.

Best uses: Standard power regulations, dimming LEDS, and high-frequency applications where phase alignment between multiple pins does not matter.

**Phase Correct PWM (Dual-Slope Counting)**

In Phase Correct PWM mode, the timer counts up and down, traveling from 0 up to TOP, and then counting backwards from TOP down to 0. The pin switches steas once on the way up and switches back at the exact same relative point on the way down. Because the timer must walk up and down the stairs, a full cycle takes exactly twice as many clock cycles as Fast PWM. The resulting pulses, however, are always perfectly centered relative to the middle of the timer cycle, even if the duty cycle is changed dynamically.

Best uses: Motor control and multi-phase power systems. If pulses are not centered, changing the duty cycle can cause minor timing shifts that create unwanted current spikes or electrical noise in large motor coils.

      Fast PWM (Single-Slope)
      TOP   |      /|      /|      /|
            |     / |     / |     / |
            |    /  |    /  |    /  |
      0     |___/___|___/___|___/___|_
                (Instant Reset)

      Phase Correct PWM (Dual-Slope)
      TOP   |      /\      /\      /\
            |     /  \    /  \    /  \
            |    /    \  /    \  /    \
      0     |___/______\/______\/______\_
                (Counts Back Down)

Reference table  
| Feature | Fast PWM | Phase Correct PWM |
| --- | --- | --- |
| Counting Mode | Up only (single-slope) | Up, then down (dual-slope) |
| Max Frequency | Highest (e.g. 62.5 kHz at 8-bit) | Half speed (e.g. 31.25 kHz at 8-bit) |
| Pulse Alignment | Left-aligned (Edges match) | Center-aligned (mids match) |
| Primary Use | LEDs, buzzers, standard switching | BLDC motors, H-bridges, audio |

### Arduino Mega Hardware

The microcontroller houses six independant internal timers. Changing the register config of a timer will never randomly bleed into or alter the PWM frequency of unrelated timers. The pin mapping for the 15 PWM-capable pins on the Mega are:

- Timer 0 (8-bit): Pins 4 and 13

- Timer 1 (16-bit): Pins 11 and 12

- Timer 2 (8-bit): Pins 9 and 10

- Timer 3 (16-bit): Pins 2, 3, and 5

- Timer 4 (16-bit): Pins 6, 7, and 8

- Timer 5 (16-bit): Pins 44, 45, 46

Manually overwriting the config registers for Timer 1 (`TCCR1A`, `TCCR1B`, `ICR1`, `OCR1A`) will directly shift the frequency or behavior of both Pin 11 and Pin 12 simultaneously. The frequency of Pin 11 cannot be changed without changing the frequency of Pin 12.

**Why Changing One Timer Can Break Other Pins or Software**

While modifying one timer won't touch the hardware registers of others, it can still break overall code:

Shared timers via core software functions and libraries:

  The Arduino software environment maps core language functions to specific hardware timers. If timer's speed or counting limit is modifyed for a custom PWM frequency, any build-in functions mapped to that same timer will completely break. Timer 0, for example, governs `millis()`, `delay()`, and `micros()`; Timer 2 governs `tone()`, rendering buzzers and audio outputs inaccurate; `Servo.h` uses Timer 5 on the Arduino Mega (Timer 1 on the Uno); etc.

| Register Name | Register | Function |
| ---           | ---      | ---      |
| `TCCR1A`      | A        | Sets PWM mode and pin output behavior (COM bits and WGM bits) |
| `TCCR1B`      | B        | Sets counting mode (WGM bits) and prescaler (CS bits) |
| `ICR1`        | ICR      | Sets the TOP value -> controls period -> controls frequency |
| `OCR1A`       | A        | Sets the compare match point -> controls duty cycle on pin 11