# Wheatstone Bridge & Thermistor

A **Wheatstone bridge** is a precise electrical circuit used to measure an unknown electrical resistance by balancing two legs of a bridge network. It consists of four resistors arranged in a diamond shape, a voltage source, and a null detector (galvanometer), allowing for highly accurate measurements of small resistance changes.

Galvanometers are very sensitive devices that can detect microamps. Most sources of information on Wheatstone bridges demonstrate classic use cases of detecting balance/imbalance in the bridge and expect a very fine resolution at fractions of mV. This project can expect changes of ~500Ω per °C/~55mV per °C which can easily read with an acceptable resolution by the arduino's ADC.

The Wheatstone bridge was invented in 1833 specifically because galvanometers existed. The bridge turns an unknown resistance measurement into a null-detection problem, which galvanometers excel at. The null point approach makes the measurement independent of source voltage stability, which was important in an era before precision voltage references existed.

**Key fundamentals**

  - Circuit Structure: The bridge consists of four resistors($$R_1, R_2, R_3, R_x$$) arranged in a diamond, with a voltage source connected across one pair of nodes and a galvanometer across the other.
  - Balance Condition: The bridge is considered "balanced" when no current flows through the galvanometer, indicating the voltage at both midpoints of the bridge is identical.
  - Formula: When balanced, the ratios of the resistances in the two legs are equal, defined by the equation:
  $${R_1 \over R_2} = {R_3 \over R_x}$$
  If $$R_1, R_2, \text{and } R_3$$ are known, the unknown resistor, $$R_x$$, can be calculated.
  - Measurement Principle (Null Detection): Instead of measuring absolute voltage or current, the bridge measures the null point (zero current), making it highly precise and independent of the source voltage stability.

  In a balanced bridge, the Galvanometer reads zero because there is no potential difference between the two points of contact. Otherwise, current flows from the side with the higher potential to the side with the lower potential. 

          Vcc (+)

             |
             |
      -------*-------

     |               |
     |               |
     R1              R3

     |               |
     |-------G-------|  <-- Galvanometer (G)
     |               |
     R2              Rx (Unknown)

     |               |
     |               |
      -------*-------

             |
             |
          GND (-)

In the above configuration:
  - Current moves **towards $$R_x$$**: If $$R_x$$ has lower resistance, pulling current across the galvanometer toward $$R_x$$.
  - Current moves **towards $$R_2$$**: if $$R_x$$ has higher resistance, pushing current across the galvanometer toward $$R_2$$.

The bridge is essentially two parallel voltage dividers. The voltage at the junction of $$R_1 \text{and } R_2$$ ~  ($$V_a$$) and the junction of $$R_3 \text{and } R_x$$ ($$V_b$$) is determined by the ratios of the resistors:

$$V_a = V_{cc} \cdot {R_2 \over {R_1 + R_2}}$$

$$V_b = V_{cc} \cdot {R_x \over {R_3 + R_x}}$$

Two voltage dividers are superior to one for sensing because they enable differential measurement. Using one active divider and one reference divider is how the bridge measure the ratio rather than the absolute voltage. A single voltage divider measures an absolute voltage, which is not as sensitive as a full bridge and easily ruined by fluctuations.

System sensitivity is highest at the balance point. Mathematically:

$$\text {Sensitivity} = {dV_b \over dR_x} = V_{cc} \cdot {R_3 \over (R_3 + R_x)^2}$$

Key details from the above equation:
- Increasing the voltage increases sensitivity.
- When $$R_x$$ is the independent variable, the maximum sensitivity is where $$R_x$$ trends towards zero (assuming no negative value components).
- When $$R_3$$ is the independent variable, there is a maximum at the constant value used for $$R_x$$.

NTC's resistance changes exponentially. Tracking becomes increasingly difficult the further from the balance point.

**Resistor Tolerance** precision isn't necessary for this project. In reality, 5% carbon film resistors are more susceptible to temperature changes from the environment and self-heating than 1% metal film resistors, unmatched resistors would create an unbalanced bridge and a potential massive offset voltage at base temperature, and 1% resistors are highly stable after thousands of hours of use.

For this project 5% resistors are good enough. Ambient temperature is relatively constant and self-heating can be assumed negligible. The resistors are measured and the software can account for the hardware error.

## Differential Voltage Measurement

The distinction between differential and single-ended measurements refers to how the bridge's output voltage is measured relative to a reference point, which directly impacts accuracy, noise immunity, and complexity. The difference between the two midpoint nodes, ($$V_a \text{and } V_b$$) of the bridge is observed without reference to ground.

**Key Advantage:** 
- Excellent noise rejection. Noise usually affects both legs of the bridge equally and gets cancelled out.
- High-precision for sensor applications like load cells, pressure sensors, and strain gauges, especially in electrically noisy environments.

**Single-Ended Measurements:** compares the voltage at only one of the midpoints against a common ground. It's simpler and cheaper; requiring fewer connections. It uses only one analog input channel, allowing more sensors to be connected to a data logger. It's also highly affected by noise and lacks the ability to subtract common-mode errors, such as temperature-induced changes in bridge resistance. Really only used if noise is not a concern.

$$V_a \text{nor } V_b$$ can be zero or $$V_{cc}$$ because current must flow through the top resistor before it reaches the measurement point, some amount of voltage drop must occur. It's only "possible" to measure $$V_{cc}$$ if the top resistor is a perfect conductor and measure zero if the bottom resistor was a perfect conductor or the top resistor was an open circuit.

In the project, the Galvanometer is replaced by ADC pins A0 & A1 similar to the voltage divider project. The voltage drop across the top resistor in each branch is measured by its respective pin, the voltages are converted to a numeric value through the ADC, and subtracted from one another. The difference of $$V_a \text {and} V_b$$ is positive if more current is flowing in the direction of $$R_x$$ and negative if $$R_x$$ is pushing current away.

This project does not require precise results and should not require an op-amp. A 10kΩ NTC at 25°C and β ≈ 3950K experiencing a change of 1°C should show a voltage differential of approximately 55mV. At 4.88mV per ADC step, we have more than 10 steps per °C which is acceptable. Due to manufacturing tolerances, V_diff at room temperature will rarely be exactly zero. The calibration step stores this initial offset and subtracts it from all subsequent readings, ensuring the zero-reference matches reality rather than the ideal balanced bridge. 

## NTC Thermistor

**NTC (Negative Temperature Coefficient) thermistors** are ceramic semiconductor resistors whose resistance decreases predictably as temperature rises, making them ideal for precise temperature sensing and inrush current limitation. They are highly sensitive, used often between -55°C and +300°C, and exhibit a non-linear exponential, rather than linear, resistance-temperature curve.

**Key Fundamentals:**

  - Operating Principle: As temperature increases, the resistance of an NTC thermistor decreases, allowing more current to flow.
  - Sensitivity: NTC thermistors are very sensitive to small temperature changes, with temperature coefficients of resistance about 10 times higher than metals.
  - Material: Composed of sintered metal oxides (such as manganese, nickel, cobalt, or copper) that for a polycrystalline ceramic structure.
  - Resistance-Temperature (R-T) Curve: Non-linear; the resistance drops exponentially with increasing temperature, often modeled using the Steinhart-Hart equation or a simple Beta $$\beta$$ formula.
  - Standard Reference: The standard reference temperature for NTC resistance is 25°C.
  - Non-linear: NTC Thermistors are made from semiconductor ceramic materials rather than pure metals.

  **The Beta ($$\beta$$) formula:**

  The Beta formula is an exponential equation that approximates how an NTC thermistor's resistance changes with temperature. It is highly accurate over narrow temperature ranges (e.g., 0°C to 100°C).

  **The Formula**

  $$R(T) = R_0 \cdot e^{\beta({1 \over T} - {1 \over T_0})}$$

  or 

  $${1 \over T} = {1 \over T_0} + {1 \over \beta} \text{ln}({R \over R_0})$$

  - $$R(T)$$: The unknown resistance ($$\Omega$$) at your target temperature $$T$$.
  - $$R_0$$: The base resistance at a known reference temperature $$T_0$$. For a 10k$$\Omega$$ thermistor, $$R_0 = 10,000\Omega$$.
  - $$T_0$$: The reference temperature in *Kelvin*. By industry standard, this is 25°C, which equals 298.15K.
  - $$T$$: The target temperature you want to measure or calculate, always in Kelvin.
  - $$\beta(\text {Beta Value})$$: A constant provided by the manufacturer (usually between 3000 K and 5000 K). It represents the slop of  the resistance curve. Higher $$\beta$$ values mean the thermistor is more sensitive to temperature changes.

  **Note:** Kelvin is used in this and many other thermodynamic equations for math and physics reasons:
  1. Math errors:
      - Division by zero. 0°C and 0°F are easily achievable in real world scenarios while 0K is never possible in real-world operations.
      - Eliminates negative numbers. Negative temperatures would reverse the signs in the exponent and produce incorrect calculations.
  2. Physical reality:  
      - Kelvin is an absolute scale directly proportional to kinetic molecular energy.
      - Because semiconductor resistance changes based on the actual thermal energy of electrons, the math must anchor to an absolute physical zero.

Also worth noting, rearranging the earlier formula:

$$V_b = V_{cc} \cdot ({R_x \over (R_3 + R_x)})$$

to 

$$R_x = R_3 \cdot {V_b \over (V_{cc} - V_b)}$$

gives the resistance of the thermistor.