## Pulse width modulation (PWM)

Pulse Width Modulation (PWM) is a digital technique used to mimic analog behavior by rapidly switching a digital signal (HIGH/LOW) at a high frequency. Varying the proportion of "on-time" (known as the duty cycle) to control average power. It is used instead of true analog output because it is highly energy-efficient, generates far less heat, and is directly compatible with modern digital microcontrollers.

**Note**: Some microcontrollers have an onboard digital-to-analog converter (DAC) to output a true analog signal in order to control analog devices or communicate with an external DAC. DACs are relatively expensive to produce and it takes up a lot of silicon area. 

A PWM duty cycle represents the percentage of time a digital signal is in the **HIGH** state compared to its total period; determining the average power delived to a load.

### Duty Cycle

Duty cycle is calculated as:

$$
D = {T_{on} \over {T_{on} + T_{off}}} \times 100
$$

A 100% duty cycle means the signal is always on while 50% means that it is on for half the time. The Arduino function, `analogWrite()`, supports values from 0 to 255, where passing 0 represents 0% duty cycle and 255 represents 100% duty cycle. Therefore $$PWM_{value}$$ can be obtained with a simple lerp:

$$PWM_{value} = {(\text{duty cycle\%}) \over 100.0} \times 255$$

`analogWrite()` is also not true analog output. It produces a digital square wave while rapidly switching from 0V to 5V. It simulates an analog voltage by varying the average "on" or "HIGH" time of a digital signal.

Average Voltage is calculated as:

$$
V_{avg} = D \times V_{supply}
$$

### Frequency

Frequency is another primary component that defines PWM's behavior. It represents the number of times a signal repeats per second. The required Hertz depends on the application. An LED controlled by a duty cycle of 20% at 1Hz will be noticeable to the human eye.

$$
f = {1 \over T}
$$

Where:

$$f = \text{frequency (Hertz, Hz)} $$

$$T = {T_{on} + T_{off}} = \text{period (seconds).}$$

Pins 2 through 13 support PWM with default frequencies:
  
  - 490Hz on most pins
  - 980 Hz on pins **4** and **13**

### RC Low-Pass Filter

An RC low-pass filter in a PWM system is primariliy used to convert square wave signals into smooth, stable analog DC voltage. It averages the PWM duty cycle, allowing low-frequency DC components to pass while attenuating high-frequency pulses. **The resistor** limits the rate at which the capacitor charges and discharges, smoothing the square waves.

**Key Purposes**

  * Converting PWM to Analog (DAC)
  * Smoothing Output Voltage
  * Reducing High-Frequency Noise

The Analog-to-Digital Converter (ADC) reads a value close to the $$V_{avg}$$ after filtering  because the filter acts as an average. The filter removes the high-frequency switching components of the PWM signal, leaving behind the DC component, which is proportional to the duty cycle.

**Cutoff Frequency**: The filter is designed with a cutoff frequency that is much lower than the PWM switching frequency. This is to ensure efficient conversion to DC:

$$
f_c = {1 \over {2\pi RC}} = {1 \over {2\pi \tau}}
$$

The PWM switching frequency ($$f_{sw}$$) must be much greater than the control loop bandwidth ($$f_c$$) or the desired signal bandwidth to allow for **clean filtering** because it separates the high-frequency switching noise from the low-frequency control signal. This allows a simple filter to produce a smooth, DC-like output with minimal ripple.

Keeping $$f_{sw} \gg f_c$$ provides the following advantages for filtering:

   * **Smaller Filter Components:** Higher switching frequencies allow for smaller inductors and capacitors to effectively filter out the switching components, reducing the size and cot of the converter.
   * **Reduced Ripple Voltage/Current:** The amplitude of the ripple is inversely proportional to the switching frequency. A higher frequency means the signal is chopped more rapidly, giving the load less time to react to the switching pulses, resulting in a smoother output.
   * **Effective Low-Pass Filtering:** The goal of the filter is to pass the desired control signal (low frequency) and block the PWM carrier frequency (high frequency). A large separation ($$f_{sw} \gg f_{cut-off}$$) enables effective attenuation of the noise.
   * **Improved Control Stability:** When the sampling frequency (related to $$f_{sw}$$) is much higher than the bandwidth, the digital controller can update the duty cycle rapidly enough to follow changes in the system without causing instability or limit-cycling.

Typically, the switching frequency is chosen to be at least 10 to 20 times the required control bandwidth. For motor control, the frequency must be high enough so the electrical time constant of the motor windings acts as a low-pass filter on the PWM signal. The switching frequency is often chosen above the human hearing range ($$> 20$$ kHz) to avoid acoustic noise.

If $$f_{sw}$$ is too close to $$f_{c'}$$ the filter cannot effectively separate the signal from the noise resulting in excessive ripple and poor performance.

**Ripple** is the small, unwanted residual AC voltage or current fluctuation remaining on top of the desired DC average output caused by the constant switching on/off of the signal. It is never zero because perfect filtering requires infinite components and the nature of switching inherently creates charging/discharging cycles.

PWM functions by rapidly turning power on and off. The instantaneous voltage causes a sawtooth-like fluctuation in current/voltage. A low-pass filter is used to smooth the PWM signal. To achieve zero ripple (eliminate all high-frequency switching harmonics completely) requires an infinite inductor or capacitor.

### Relationship between $$\tau$$ and $$f_c$$

The time constant ($$\tau = RC$$) and the cutoff frequency ($$f_c = {1 \over {2\pi\tau}}$$) are inversely related, representing two different ways to describe the same filter performance. Increasing $$\tau$$ (slower, more stable) directly results in a lower $$f_c$$ (better filtering). The tradeoff is that increasing $$\tau$$ blows down the system's reaction to changes in the duty cycle.

## LED Behavior under PWM

An LED responds to average power rather than instantaneous power primarily because the process of producing light is too fast for the human eye to track. The eye perceives an average brightness based on the duty cycle rather than the flickering at the instant of power delivery.

PWM dimming is preferred over resistor-based (analog) dimming because it maintains consistent color temperature, offers higher energy efficiency, and allows precise brightness control without overloading components. By switching the LED on and off rapidly, PWM operates LEDs at their optimal current, reducing heat generation and preventing the color shifts that occur when reducing current via resistors.

The forward voltage drop ($$V_f$$) of a red LED (~1.8-2.2V) is a critical threshold that makes resistor-based dimming particularly difficult to control compared to PWM. LEDs are non-linear devices. Once you hit the $$V_f$$, the relationship between voltage and current is extremely steep. A tiny increase of just 0.1V above the forward voltage can double or triple the current. Attempting to set the voltage in a very narrow range just about the $$V_f$$ will result in the LED shutting off entirely if the voltage drops below the diode turn on voltage.

The current limiting resistor formula: 

$$R = {(V_{supply} - V_f) \over I_f}$$

where $$I_f = \text{Desired forward current}$$

is an application of Ohm's Law designed to determine how much resistance is needed to "choke" the power supply's current down to a safe level for the LED. A LED acts as a "constant voltage" sink that consumes its forward voltage first leaving the resistor to handle whatever voltage remains.

**Worked Example**

   * Supply Voltage ($$V_{supply}$$): 5V
   * Forward Voltage ($$V_f$$): 2.0V
   * Desired Current ($$I_f$$): 20mA (0.02A)

$$R = {{5V - 2.0V} \over {0.020A}} = {{3V}\over{0.020A}} = 150\Omega$$

Ensuring the resistor can handle this without burning out:

$$P = I^2 \times R$$ or $$P = V_{resistor} \times I_f$$

$$P = 3V\times0.020A=0.06W$$

For a standard 1/4 Watt resistor, this is more than sufficient.

### Timer conflicts with Arduino libraries

In the Arduino world, timers are the hardware "clocks" inside the microcontroller that generate PWM signals. Since each timer is shared by multiple pins, using a library that takes control of a timer can disrupt or completely disable PWM on its associated pins (e.g. Servo library uses Timer 1. Timer 1 is mapped to pins 9 and 10. `analongWrite()` can no longer be used on those pins and they stop behaving as PWM pins).

### Sources of Error

While the ideal reading for filtered PWM signal is $$V = D \times V_{Supply}$$, several factors introduce errors that cause the ADC reading to deviate from this value.

   1. Residual Ripple
      * The low-pass filter does not perfectly remove all AC components; it only attenuates them. A residual "sawtooth" or "sine-like" oscillation remains on the DC signal. If the ADC samples the signal at a random point in the ripple cycle, the reading will be slightly higher or lower than the true average.

   2. ADC Quantization Error
      * ADCs convert continuous analog voltages into discrete digital steps, which inherently introduces a rounding error. The maximum quantization error is typically $$\pm0.5$$ LSB (Least Significant Bit).

   3. Filter Settling Time
      * The filter requires a finite amount of time to respond to changes in the PWM duty cycle. When the duty cycle changes, the capacitor must charge or discharge to the new average voltage level. This follows the $$RC$$ time constant. If the ADC samples the signal before the filter has "settled" (5 to 7 time constants for high precision), the reading will reflect a transitional value rather than the new steady state average. Stronger filtering reduces ripple, but increases the settling time, making the system slower to respond to duty cycle updates.

**To verify that the filter is effectively converting a PWM signal into a DC voltage,** you can perform a series of tests to confirm it accurately tracks the duty cycle.

1. Static Linearity Test (DC Accuracy)
   - The most direct way to verify tracking is to sweep the duty cycle and compare the ADC readings to the theoretical $$V = D \times V_{Supply}$$.
      * Set the PWM to specific duty cycle increments (e.g. 0%, 25%, 50%, 75%, 100%).
      * Measure the output of the filter with a digital multimeter set to DC volts.
      * If the filter is working the multimeter should increase linearly with the duty cycle. Compare this physical measurement against you ADC digital output to identify any calibration offsets.

2. Ripple Analysis (AC Verification)
   - If the filter is undersized, the ADC ay see a "bouncing" value due to residual AC ripple.
      * Using an oscilloscope in "AC Coupling" mode to zoom in on the filtered DC sginal.
      * Check the peak-to-peak ripple voltage. For a 10-bit ADC, the ripple should idealy be less than 1 LSB (approx. 4.8mV for a 5V system) to ensure stable readings. 
      * If the ripple is too high, the PWM frequency and/or $$RC$$ constant need to increase.

3. Step Response Test (Settling Time)
   - This verifies how quickly the ADC can "catch up" to a new duty cycle value.
      * Program the pw to jump instantly from 10% to 90% duty cycle.
      * Observe the output on an oscilloscope. The time it takes for the voltage to reach and stay within its new final value is the settling time
      * Log ADC readings at a high rate during the transition. 

4. Software Correlation
   - Automate the verification by printing a table to your serial monitor.
      * Column A: Commanded Duty Cycle (%).
      * Column B: Theoretical Voltage ($$V_{target}$$).
      * Column C: Actual ADC Reading ($$V{measured}$$).
      * Result: Plotting these in a spreadsheet should produce a straight line with a slope equal to the supply voltage



























