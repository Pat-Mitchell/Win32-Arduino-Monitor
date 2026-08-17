# Arduino

When using an Arduino to process audio or AC signals (like from a microphone or guitar pickup), there is a critical voltage compatibility barrier solved by using single-supply biasing.

## Arduino ADC limitations

The Arduino's ADC can only measure voltages between 0V and 5V. Real AC signals or audio waves swing symmetrically above and below 0V into negative territory. Feeding raw AC signal directly into an Arduino will either clip the signal at 0V or cause damage to the microcontroller chip.

To fit a bidirectional AC signal into a stricly positive 0-5V window, the baseline must be shifted. Two equal-value resistors (typically 10kΩ to 100kΩ) are placed inseries from the Arduino's 5V rail to GND. The middle node between these resistors creates a mid-supply reference voltage: $V_{bias} = 2.5 \text V$. This 2.5V reference connects directly to the op-amps's non-inverting ($V_+$) input. Because of the virtual short, the entire op-amp circuit now treats 2.5V as its new virtual ground reference.

## AC-Coupled Input

Raw AC signal cannot be directly connected to the biased input or the signal source will pull the 2.5V bias back down to ground. An input coupling capacitor is placed in series between the signal source and the op-amp input. The capacitor blocks the DC bias from the sensor and allows AC frequencies to pass, allowing the signal wave to ride on top of the 2.5V DC baseline.

Because the baseline is centered at 2.5V, the amplified output signal expands and contracts symmetrically around this middle point. The positive peak is 5V and the negative peak is 0V; causing the ADC to read 512 at the baseline, 1023 at the positive peak, and 0 at the negative peak. The entire wave can be captured.

### Bypass Capacitor

Any noise on the Arduino's 5V powerline (caused by microcontrollers, digital switching, or LEDs) will bleed into the 2.5V bias divider. Connecting a small (10μF) capacitor in parallel with the bottom resistor to act as a low pass filter and dampen rapid voltage spikes and ripples from the power supply so the 2.5V bias remains stable under load.

### Coupling Capacitor

A capacitor is placed in series directly between the signal source and the input resistor ($R_{in}$) of the op-amp. Most signal sources carry an underlying DC voltage (electret microphones require a constant DC bias voltage to operate). Connecting them directly to the op-amp (LM358P) destroys the symmetrical baseline. The capacitor, acting as a high pass filter, prevents DC current from passing while allowing AC signals to pass.

The boundary point is defined by teh cutoff frequency formula:

$$f_{cutoff} = {1 \over 2 \pi \times R_{in} \times C_{coupling}}$$

Using a 10kΩ resistor and 1µF capacitor pair yields:

$$f_{cutoff} = {1 \over 2 \times \pi \ 10 \text k \Omega \times 1 \mu \text F} \approx 16 \text {Hz}$$

16 Hz is below the lowest threashold of human hear (20 Hz), so this specific pairing enusres that the entire audio spectrum passes into the Arduino Mega completely intact.

### Decoupling Capacitor

Decoupling capacitors (or bypass capacitors) act as local energy reservoirs to ensure the op-amp (LM358) receives stable, noise-free power at the power pins. The recommended capacitor is a 0.1µF ceramic capacitor. Specifically, ceramic capacitors have very low equivalent series resistance (ESR) and low inductance. They can respond instantly to high-frequency noise and voltage spikes faster than electrolytic capacitors.

In order to work properly, the decoupling capacitor must be placed as physically close as possible to the LM358's power pin (pin 8, V+) and ground pin (pin 4). Long wires and breadboard tracks act like tiny antennaas and inductors, picking up stray electromagnetic interference (EMI) from the environment. If the capacitor is placed far away, the resistance and inductance of the intervening wires will counteract its ability to suppress high-frequency noise.

## Op-Amps LM358 and TL071

### Headroom

The LM358 is not a rail-to-rail op-amp. While its output can swing completely down to the negative rail (0V), it upeer internal transistors require headroom. It cannot push past rought $V_{cc} - 1.5 \text V$. With the Arduino's 5V rail, the maximum output voltage is only 3.5V. Using the 2.5V bias point, the signal can only swing 1.0V up (2.5V to 3.5V) before hard-clipping. However, it can swing 2.5V down (2.5V to 0V).

### Slew Rate

The slew rate defines how fast the op-amp's output can physically change its voltage over time.

**The LM358**'s slew rate is 0.3V/µs (volts per microsecond). A high frequency wave (fast audio transient) demanding the output of the op-amp to jump instantly from low to high peaks will deform clean sine waves into triangle waves

**The TL071** has a slew rate of 13 to 20 V/µs. By comparison, it tracks fast audio transients without altering the waveform shapes.

### Audio signals

To keep power consumption incredibly low, the designers of the LM358 starved its internal output stage of idling current. The LM358 uses a Class-B output structure. When an audio wave passes through the 2.5V baseline (switching from pushing sourcing to sinking current), there is a tiny "dead zone" where neither output transistor is fully active. The result is a distinct notch or flat spot exactly in the middle of the audio wave. The crossover distortion sounds like a harsh, fuzzy crackle on sustained notes. The TL071 uses a much cleaner Class-AB architecture that eliminates this glitch.

## Running on Arduino 5V

The TL071's superior speed and fidelity would be mostly unusable on a single 5V Arduino power rail

| Feature | LM358 | TL071 | Why is matters |
| --- | --- | --- | --- |
| Min. supply voltage | 3.0V | 4.5V to 10V (depending on the model) | The TL071 is starved for voltage at 5V and will exhibit unstable behavior. |
| Input range at 5V | 0V to 3.5V | Requires 4V from rails | A TL071 requires the input to stay far away from ground. At 5V, its valid input window disappears. | 
| Output swing at 5V | 0V to 3.5V | Locks up/Saturated | The TL071 output will slam into a rail and fressze because it lacks dual voltage tracks. | 
| Slew Rate | 0.3 V/µs (slow) | 13-20 V/µs (fast) | The TL071 wins on speed, but only if given a proper $\pm 9V$ or $\pm 15V$ power supply. | 

## Fixing the headroom clipping issue after discovering the limitations of the L358

The 2.5V bias needs to be 1.75V for a midpoint between 0V and 3.5V. The resistors in the voltage divider need to have a ratio of approximately 1.857 to give the desired voltage. Resistor choices are 18kΩ and 10 kΩ for a bias of 1.78V or a more accurate 22kΩ and 12kΩ for a bias of 1.76V.

The new values the arduino ADC will read at a baseline with no signal is 358 to 364.

## ADC fast sampling

Arduino ADC fast samping allows for the bypass of the standard conservative speed limits of the microcontroller to capture rapidly chaning signals like audio or low-frequency radio waves. By modifying the hardware ADCSRA (ADC Control and Status Register A) register, the division factor (prescaler) that dictates how fast the ADC clocks its cycles can be changed.

By default, the prescaler is set to /128 (~9.6 kHz). It balances noise rejection with speed. It yields an ADC clock of 125 kHz, taking roughly 104 microseconds per sample. Pushing the hardware to /16, /8, or /4 pushes the accuracy limits of the hardware with faster clockrates. For basic audio processing, /4 (~307.7 kHz) results in noisey data as the register does not have time to settle and bit resolution drops below 10 bits.

Dropping the prescaler below 32 shows the limits of the Arduino hardware. The internal sampling capacitor has significantly less time to charge up and the effective number of bits drops (only 7-8 stable bits of resolution instead of the full 10).

### Nyquist limit

To avoid aliasing (where a high-frequency signal masquades as a lower frequncy), the sampling rate must be at least double the highest frequency component of the signal. I.e. to capture a 20 kHz audio frequency, the sampling rate must be above 40 kHz. Ideally, 10 to 20 samples per period provide enough resolution to cleanly visualize, measure peak to peak voltages, or execute accurate fast fourier transforms without severe distortion.

## Oscilloscope Trigger

An oscilloscope trigger actls like a camera shutter synchronized to a moving object. A fast-moving electrical signal looks like an unreadable blur of overlapping lines without it. Conversely, an untriggered display rolls continously across the screen from left to right to graph voltage over time. 

Triggers forces the oscilloscope to wait until the signal meets a certain condition before it draws the data. This ensures that every frame of data appears in phase and frozen in place on the screen. Typically trigger types are rising edge where the scope waits until the signal crosses a designated voltage threshold while moving upward or falling edges where the scope waits until the signal crossed a designated threshold while moving downward.

With old analog oscilloscopes, a trigger could only show what happed after the trigger eveny occured. Modern digital storage oscilloscopes continously stream data into a memory buffer. When the trigger event happes, the scope displays a pre-trigger buffer that shows data from before the trigger event occurs.