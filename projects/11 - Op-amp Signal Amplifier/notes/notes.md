# Op-Amp Signal Amplifier

An op-amp (operational amplifier) is an electronic chip that makes weak electrical signals bigger. It has two inputs and one output. It works by measuing the difference between the two inputs and multiplying that value.

- Non-inverting Input (+): The positive input pin. A signal here keeps the same phase on the output.

- Inverting Input (-): The negative input pin. A signal here flips the phase of the output by 180 degrees.

- Output: The final amplified voltage signal.

- Power Supply: Pins for positive and negative DC voltage to power the chip.

- Gain: The measure of how much the op-amp multiplies the input signal.

## Ideal Op-Amp Characteristics

An ideal op-amp is a theoretical model used to simplify circuit design and analysis. While real-world op-amps have physical limitations, "ideal" op-amps make calculating circuit behavior incredibly easy. The core properties of an ideal op-amp and their real-world counterparts shaper how these components interact with electronic signals:

- Infinite open-loop gain: The raw chip amplifies the differential signal by a massive factor.

- Infinite input impedance: It draws virtually zero current into its input terminals without distortion or "loading down" the incoming sensor signal.

- Zero output impedance: It can supply as much current as needed to downstream components without losing voltage strength.

- Infinite bandwidth: The chip can amplify signals of any frequency from 0 Hz (DC current) to infinitely high radio frequencies with the exact same level of gain and zero time delay (phase shift).

### Real-World Op-Amps

**Op-amps' input impedance is finite** and allows small leakage currents to flow into or out of the input terminals. Real world op-amps are restricted by the semiconductor architecture used to build their internal input stages. The inputs are connected to the control gates or bases of a differential transistor pair. Depending on the type of transistors used, the input impedance behaviors change significantly:  

  - In standard op-amps, the input stage relies on base-emitter junctions. Because bipolar junction transitors (BJTs) require a continuous base current to operate, the input impedance is relatively low (1 MΩ and 1 MΩ).  
  - Modern high-impedance op-amps use JFETs or MOSFETs for their input stages. Since these gates are isolated by a layer of oxide or a reverse-biased junction, the input impendance is massive (1 GΩ to 1 TΩ).

While FET-input op-amps get incredibly close to "infinite" impedance, they still experience minuscule leakage currents that scale up dramatically with temperature. The timy current flowing though external external circuit resistors creates uninteded coltage drops that cause errors and shifts in the final output signal.

**Op-amps do not have infinite gain at any frequency.** Real op-amps have a large but finite open-loop DC gain (100,000 to 1,000,000) and this gain drops steadily as the signal frequency increases due to internal capacitance. The product of the open-loop gain and the frequency is a constant value: Gain-Bandwidth Product (GBW). If an op-amp has a GBW of 1 MHz, its gain drops to 1 at 1 MHz.As open-loop gain drops at higher frequencies, the circuit has less loop gain left to stabilize the output, leading to distortion and timing errors.