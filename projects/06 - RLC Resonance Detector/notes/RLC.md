# RLC Resonance Detector

A PWM signal is swept across a frequency range and fed into a series RLC circuit. The Arduino measures voltage across the sense resistor at each frequency step, streams the data to a Win32 app, which plots the frequency response curve and identifies the resonant peak.

## Inductors

An inductor is a passive electronic component that temporarily stores electrical energy as a magnetic field. Whenever electrical current flows through a wire, it creates a magnetic field. Coiling the wire concentrates this field, making the inductor highly effective at resisting sudden changes in current. When current increases, the inductor generates an opposing voltage (back EMF) to slow the surge. When current decreases, it releases its stored magnetic energy, pushing electrons to keep the current flowing.

### Key Characteristics

- Inductance ($L$): The measure of an inductor's ability to store energy. It is measured in Henries (H) and is determined by the coil's shape, number of turns, and core meterial.

- Energy Storage ($E$): Calculated using the formula $E = {1 \over 2} LI^2$, where $L$ is the inductance and $I$ is the current.

- Core Materials: Air cores are used for high frequencies, while ferromagnetic cores are used to significantly increase inductance. 

- In **DC** Circuits: When the circuit is first turned on, the inductor opposes the current. Once the magnetic field stabalizes, it acts simply like an ordinary wire.

- In **AC** Circuits: Inductors block or "choke" high-frequency alternating current while allowing low-frequency or direct current to pass easily. The opposition to AC is called inductive reactance ($X_L$), measured in ohms.

- Real inductors have a small series DC Resistance (DCR). DCR effectively increases the totalt resistance of the circuit and reduces the Q-factor. Typically, it's around 1–5Ω for a 100µH axial inductor.

### Common Applications

- Filters: To block high-frequency noise and smooth out ripples in power supplies.

- Energy Reservoirs: To temporarily store and release energy in voltage converters and regulators.

- Tuners: Combined with capacitors in LC "tank" circuits to tune into specific frequencies in radios and wireless communications.

## Faraday's and Lenz's Laws

**Faraday's Law** dictates how much voltage an inductor generates, while **Lenz's Law** dictates the direction of the voltage to oppose changes in current. Together, the two principles form the foundation of electromagnetic induction and explain why inductors resist sudden changes in electrical current.

### Faraday's Law: Generating the Voltage

Faraday's Law states that any change in the magnetic environment of a coil of wire will cause a voltage, known as an electromotive force (EMF), to be induced in the coil.

When current passes through an inductor, it creates a magnetic field. If the current changes, the magnetic field expands or collapses. This changing magnetic field cuts through the inductor's own coils.

The formula:

$$\mathcal{E} = -N {\Delta \Phi_B \over \Delta t}$$

Where:  
$\mathcal{E}$ is the induced voltage  
$N$ is the number of wire turns  
$\Delta \Phi_B \over \Delta t$ is the rate of change of the magnetic flux.

In an inductor, it simplifies to  
$$\mathcal{E} = -L {\Delta I \over \Delta t}$$
This tells us that the faster you try to change the current ($\Delta I \over \Delta t$), the more voltage the inductor will generate to push back.

### Lenz's Law: Directing the Pushback

Lenz's Law determines the direction of the induced voltage and acts as the physical manifestation of the law of conservation of energy. It states that the induced current will always flow in a direction that opposes the change in magnetic flux that created it. In Faraday's equation, Lenz's law is represented entirely by the negative sign.

It acts as electrical intertia. If you try to increase the current through an inductor, Lenz's law dictates that the induced voltage will push against the incoming current. Likewise, decreasing or shutting off the current results in the induced voltage gradually fading.

The mechanical equivalent of an inductor would be a flywheel. Both components function as energy storage devices that rely on interia to opposed sudden changes in a system. To continue the tangent, the equations for stored energy and force required to change state are nearly identical:

Flywheel:  
$E = {1 \over 2} I \omega^2$ and $\tau = I {\Delta \omega \over \Delta t}$

Inductor:  
$E = {1 \over 2} L I^2$ and $V = L {\Delta I \over \Delta t}$

## Quality Factor

The Quality Factor (Q-factor) is a dimentinoless parameter that measures how "good" or efficient an RLC circuit is at storing energy compared to how much energy it wastes. A high Q-factor means the circuit holds onto its energy with very little dampening while a low Q-factor means the circuit loses energy rapidly. In an RLC circuit, inductors and capacitors store adn swap energy back and forth while resistors permanently dissipate energy as heat.

The formula for Q depends on how the components are arranged in the circuit:

- **Series** RLC circuit - a smaller resistance leads to less energy loss, resulting in a higher Q:

$$Q = {1 \over R} \sqrt {L \over C}$$

- **Parallel** RLC Circuit - The relationship flips. A larger resistance forces current through the storage components instead of the resistor, resulting in a higher Q:

$$Q = R \sqrt {C \over L}$$

The Q-factor directly dictates how a circuit responds to different AC frequencies.

High Q-Factor (e.g., Q > 10):  
- Underdamped: The circuit will ring or oscillate for a long time when disturbed. Just like a high-quality tuning fork or a heavy flywheel with perfect bearings
- Sharp Selectivity: In a radio reciever, a high Q means the circuit has a very narrow bandwidth. It can sharply tune into one specific radio station while completely blocking neighboring stations.

Low Q-Factor (e.g., Q < 0.5):  
- Overdamped: The circuit will not oscillate. It squashes signals quickly. Acting like a flywheel hooked up to a heavy fluid brake.  
- Broad Bandwidth: The circuit responds to a wide range of frequencies rather than targeting a single specific one. Audio speakers need to treat a wide range of frequencies (bass and treble spectrum) relatively equally rather than aggressively amplifying just one single note.

### Relation to Resonate Frequency

The Q-factor determines how tightly the circuit focuses around its resonant frequency ($f_0$). It acts as a mathematical bridge linking the circuits's ideal tuning point to its actual operating bandwidth($B$). In any resonant RLC circuit, the relationship is defined by the equation:

$$Q = {f_0 \over B}$$

Where:
- $f_0$ is the resonany frequency.
- $B$ is the bandwidth (the range of frequencies where the circuit operates efficiently).

The bandwidth represents the width of the frequency peak. It is measured between the two half-power points (also called the -3 dB points). At these precise frequencies, the power drops to exactly 50% of the peak value. The lower and upper frequency cutoffs ($f_1$ and $f_2$) can be calculated by:

$$B = f_2 - f_1 = {f_0 \over Q}$$

### Example

Imagine a series RLC circuit designed to resonate at $f_0 = \text {10,000 Hz}$

- Scenario A (High Q): If the components give $Q = 50$:

$$B = {\text {10,000} \over \text {50}} = \text {200 Hz}$$

The circuit only accepts frequencies from roughly 9,900 Hz to 10,000 Hz.

- Scenario B (Low Q): Swap in a larger resistor and drop the quality down to $Q = 2$:

$$B = {\text {10,000} \over \text {2}} = \text {5,000 Hz}$$

The circuit responds to a wider range of frequencies from 7,500 Hz to 12,500 Hz.

## Voltage Divider at Resonance

At resonance, an RLC circuit behaves like a simple, purely resistive voltage divider because the inductor and capacitor cancel each other out mathematically. However, depending on where the output voltage is measures, there is a massive voltage magnification effect equal to the Q-factor.

### The Total Circuit Impedance Drops to R

A standard series RLC circuit acts as a voltage divider between the total circuit impedance($Z$) and whichever component chosen to measure across. The total impedance formula for a series RLC circuit is:

$$Z = \sqrt {R^2 + (X_L - X_C)^2}$$

At the resonant frequency:

- The inductive reactance ($X_L = 2 \pi f_0 L$) and capacitive reactance ($X_C = {1 \over 2 \pi f_0 C}$) are exactly equal.

- They cancel each other out entirely: $X_L - X_C = 0$.

- Therefore, the total impedance simplifies strictly to the resistor's value: $Z = R$.

Because the impedance is at its absolute minimum, the circuit draws its maximum possible current ($I_{max} = {V_{in} \over R}$).

### Measuring Across the Resistor ($V_{out} = V_{in}$)

A voltage divider set up to measure the output voltage across the resistor ($R$) measures: $V_R = I \times R$. Since the total impedance of the entire circuit is also $R$, the resistor drops 100% of the source voltage. The result is $V_{out} = V_{in}$. The phase shift is $0^\circ$.

### Measuring Across the Inductor or Capacitor ($V_{out} = Q \times V_{in}$)

This is where the unique properties of resonance appear. The voltage divider equation yields interesting results just measuring the output voltage across just the inductor or just the capacitor.

The voltage across the inductor ($V_L$) at resonance is:

$$V_L = I \times X_L$$

Since the current flowing through the circuit is $I = {V_{in} \over R}$, the voltage across the inductor formula becomes:

$$V_L = ({V_{in} \over R}) \times X_L = V_{in} \times ({X_L \over R})$$

Recalling that Quality Factor ($Q$) for a series circuit: $Q = {X_L \over R}$ and substituting $Q$ into the equation gives:

$$\mathcal{V}_{out} = Q \times V_{in}$$

At resonance, the voltage across the inductor (or capacitor) is magnified by a factor of $Q$. A 1 volt source fed into a high-Q circuit where $Q = 100$ will measure $100 V$ across the inductor and $100 V$ across the capacitor simultaneously.

This seemingly impossible feat of violating Kirchhoff's Voltage Law is possible because the inductor and capacitor are $180^\circ$ out of phase. Their voltages are equal and opposite ($+100V \text {and} -100V$). They completely cancel each other out in the eyes of the source, leaving only the 1 V drop across the resistor.

**Note:** The 100V would be very real and dangerous. Hobbyist level components have a voltage rating of 50V to 600V. Although suffer from higher Equivalent Series Resistance (ESR) and DC Resistance (DCR) that caps the achieveable Q-factor to a range between 10 and 100 and components would likely visibly breakdown/fail, it's worth pointing out that getting shocked is possible.