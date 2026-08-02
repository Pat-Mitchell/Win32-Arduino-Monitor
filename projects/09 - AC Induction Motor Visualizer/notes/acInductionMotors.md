# AC Induction Motors

An AC induction motor is a stype of electric motor that runs on AC current and uses electomagnetic induction to create movement instead of using direct electical connections to its moving parts. When AC power flows int hte stator, the outer stationary ring of electromagnets connected to an AC power source, it builds a magnetic field that spins in a circle. The spinning magnetic field passes over the metal bars of the inner rotor (often called a "squirrel cage" because it looks like a metal exercise wheel for a pet) and forces an electric current inside them. The cre current inside the rotor creates its own magnetic field. The two magnetic fields push against each other and makes the rotor spin to try and catch up with the outer field.

## Motor Construction

### Stator Construction

When three winding sets are placed $120^\circ$ apart around the stator bore, they form the foundation of a three-phase AC induction motor. The specific mechanical spacing matches the timing of three-phase electrical power to create a smooth, self-starting rotation.The three separate alternating currents peak $120^\circ$ apart from one another coinciding with the three winding sets around the $360^\circ$ bore.

As Phase A peaks, it pulls the rotor. A fraction of a second later, phase B peaks and pulls the rotor further. Likewise, with phase C and back to phase A. The sequential peaking creates a magnetic field that physically rotates around the inside of the stator even thoug hte stator itself is completely stationary. The moving field automatically induces current in the rotor, making three-phase motors completely self-starting without the need for extra starting capacitors or switches.

The stator core is built from thin, laminated layers of silicon steel stacked together. These laminations prevent energy loss from heat (eddy currents[[^1]](#footnote-1)). Grooves cut along the inside of the stator bore where the isulated copper wire windings are tightly packed. A heavy outer housing (cast iron or aluminum) that holds the core in place and protects the internal components.

[^1]: Circular loops of electric current induced inside conductors by a changing magnetic field or relative motion. The generate an opposing magnetic field that creates drag and heat. A common use is magnetic braking to slow trains, trucks, and roller coasters safely without physical friction. In the stator core, they are highly undesireable and a direct cause of energy waste and excessive heat.

The inner bars of the rotor require no brushes or external connections because the rotor operates entirely via wireless electromagnetic induction. The rotor does not receive electricity from an outside power supply. The rotor bars generate their own electrical current purely from the shifting magnetic fields surrounding them.

Through Faraday's law of induction, as the magnetic field generated from the stators cuts across the metal rotor bars, a voltage and electric current is induced in them. The squirrel cage completes the circuit by the end rings forming a permanently closed, short-circuited loop entirely contained within the rotor. The induced current inside the short-circuited bars creats its own magnetic field that interacts with the stato's rotating magnetic field. This produces torque that forces the rotor to spin.

The mechancial simplicity and low maintainence of AC induction motors is why it dominates over 90% of industrial manufacturing applications. The motor essentially consists of only one moving assembly (the rotor and two bearings) with not commutators or slip rings and a solid uninsulated metal bars cast directly into steel laminations. The squirrel cage design can withstand massive centrifugal forces, heavy vibrations, and sudden mechanical shocks without breaking; no exposed electricall insulation to degrade means the motor can operate in heavily contaiminated, dusty, or wet industrial environments; open electrical contacts means there is zero risk of sparking and igniting the surrounding environment (chemical plants, oil refineries, and grain silos); and solid aluminum or copper rotor bars tolerate high heat cycles during heavy startup loads. Maintainance is minimal: there is no need for brushes or commutators. The only wearing parts are the mechanical bearings that can frequently last tens of thousands of hours on standard grease schedules.

## Rotating Magnetic Field

Three-phase stator currents produce a rotating magnetic field through a perfect synchronization of spatial placement and electrical timing. The three stator windings are physically placed $120^\circ$ apart around the motor frame and energized by three AC currents that are $120^\circ$ out of phase. Their individual magnetic fields combine into a single, constant amplitude net field vector that rotates smoothly in space at synchronous speed.

### How the field rotates

To create a rotating field, the motor relies on three different phases of AC current: phase A (the reference point a $0^\circ$), phase B (offset $120^\circ$ from phase A), and phase C (offset $240^\circ$ from phase A and $120^\circ$ from phase B). Each phase supplys three independant coil windings wrapped around the stator core, spaced exactly $120^\circ$ apart around the circular core. The combination of the electrical and geometric offsets produce the rotating field.

Each individual coil can only produces a magnetic field that pulses back and forth along its own fixed physical axis. Utilizing the currents peaking at different times, the sum of the three pulsating vectors behave like a spinning arrow.

1. At $0^\circ$: Phase A peaks positively

    - Currents: Phase A is at its maximum positive value. Phases B and C are both negative and at half-strength.

    - Fields: Phase A pulls the magnetic field strongly toward the $0^\circ$ physical axis. Phase B and C push weakly away from their respective physical axes.

    - Net Vector: The net field points directly towards $0^\circ$.

2. At $120^\circ$: Phase B peaks positively

    - Currents: One-third of an electrical cycle later, Phase B reaches its maximum positive value. Phases A and C are at half-strength negatively.

    - Fields: The primary magnetic pull shifts entirely over to the Phase B coil.

    - Net Vector: The net field smoothly swings around and now points toward $120^\circ$.

3. At $240^\circ$: Phase C peaks positively

    - Currents: Phase C reaches its maximum positive value. Phase A and B drop to half-strength megative.

    - Fields: The primary magnetic pull is towards the Phase C coil.

    - Net Vector: The total net field swings further and points directly towards $240^\circ$.

While individual fields constantly grow, shrink, and reverse direction, their trigonometric combination yields a mathematical constant. Using vector addition, if $B_m$ is the peak magnetic field of a single phase, the net magnetic field ($B_{net}$) at any given instant is always 1.5 times the maximum field of an individual coil:

$$B_{net} = 1.5 \times B_m$$ [[^2]](#footnote-2)

[^2]: The time-varying magnitudes of the magnetic fields, oscillating at angular frequency $\omega$, are shifted $120^\circ$ in time:

    $$B_A(t) = B_m \cos (\omega t)$$

    $$B_B(t) = B_m \cos (\omega t - 120^\circ)$$

    $$B_C(t) = B_m \cos (\omega t - 240^\circ)$$

    The net magnetic field vector $\vec B_{net}(t)$ is found by breaking each phase into components x and y:

    $$\vec B_A(t) = B_m \cos (\omega t) \hat i + 0 \hat j$$

    $$\vec B_B = B_m \cos (\omega t - 120^\circ)(-{1 \over 2}) \hat i + B_m \cos (\omega t - 120^\circ)({\sqrt 3 \over 2}) \hat j$$

    $$\vec B_C(t) = B_m \cos (\omega t - 240^\circ)(-{1 \over 2}) \hat i + B_m \cos (\omega t - 240^\circ)(-{\sqrt 3 \over 2}) \hat j$$

    Summing all three equations and isolating the $\hat i$ components yields:

    $$B \hat i = B_m[\cos(\omega t)-{1\over2}\cos(\omega t-120^\circ)-{1\over2}\cos(\omega t-240^\circ)]$$

    Applying trig identity $\cos(\alpha-\beta) = \cos\alpha\cos\beta+\sin\alpha\sin\beta$:

    $$B \hat i = B_m[\cos(\omega t)-{1\over2}(-{1\over2}\cos(\omega t)+{\sqrt3\over2}\sin(\omega t))-{1\over2}(-{1\over2}\cos(\omega t)-{\sqrt3\over2}\sin(\omega t))]$$

    Distributing constants and cancelling out $\sin(\omega t)$ terms yields:

    $$B\hat i=B_m[1+{1\over4}+{1\over4}]\cos(\omega t) = 1.5B_m\cos(\omega t)$$

    A similar derivation is performed for the $\hat j$ component showing that:

    $$\vec B_{net}(t) = 1.5B_m\cos(\omega t)\hat i+1.5B_m\sin(\omega t)\hat j$$

    The scalar magnitude is found via the Pythagorean theorem and identity: $\cos^2\theta + \sin^2\theta = 1$:

    $$\lvert\vec B_{net}\rvert = \sqrt{B_x^2+B_y^2}$$

    $$\lvert\vec B_{net}\rvert = \sqrt{(1.5B_m\cos(\omega t))^2+(1.5B_m\sin(\omega t))^2}$$

    $$\lvert\vec B_{net}\rvert = \sqrt{2.25B_m^2(\cos^2(\omega t)+\sin^2(\omega t))}$$

    $$\lvert\vec B_{net}\rvert = \sqrt{2.25B_m^2(1)} = 1.5B_m$$

### Synchronus Speed

Because the magnitude never changes, the magnetic fields does not pulse or flicker. It glides in smooth, continuous circle at synchronous speed ($N_s$), which is dectated by the power supply frequency ($f$) and the number of stator magnetic poles ($P$).: 

$$N_s = {120 \times f \over P}$$

Where:

- $N_s$ is the synchronous speed at which the net magnetic field vector rotates around the stator core. Measured in RPM.

- $f$ is the electrical frequency measured in Hertz (Hz).

- $P$ is the total pole count or number of individual magnetic poles (North and South) wound into the stator core per phase. It's always an even integer.

- 120 is a constant factor that scales seconds to minutes and converts individual magnetic poles into pairs.

Electrical frequency serves as the "clock rate" for the magnetic field. Every single complete AC cycle pushes the magnetic vector forward through one complete pair of North and South poles. The direct relationship between frequency and RPM means means that higher frequencies translates to a faster spinning magnetic field. Modern industrial systems use Variable frequency Drives (VFDs) to alter $f$. By electronically shifting the frequency to speed up or slow down a standard motor without changing its physical hardware.

The pole count represents the physical layout of the copper windings embedded inside the steel slots of the stator fram. A pole is an electromagnet configuration. The inverse relationship means more poles yield a slower rotational speed. Because pole counts must be even numbers, fixed-frequency industrial motors can only operate at specific, discrete synchronous speeds:

| Total Poles ($P$) | Poles Pairs | Synchronous Speed at 60 Hz | Common Application Type |
| --- | --- | --- | --- |
| 2 Poles | 1 Pair | 3600 RPM | High-speed centrifugal pumps, turbo blowers |
| 4 POles | 2 Pairs | 1800 RPM | Standard industrial conveyors, fans, compressors |
| 6 Poles | 3 Pairs | 1200 RPM | Heavy rock crushers, large hoists, mixers |
| 8 Poles | 4 Pairs | 900 PRM | Low-speed direct-drive ventilation, giant stamps |

### Synchronous vs. Actual Motor Speed

AC induction motors can never actually run at $N_s$. If the solid rotor spun at the exact same speed as the magnetic field, the rotor bars would experience zero relative motion. No magnetic lines of force would be cut. Meaning zero voltage would be induced, zero current would flow, and torque would be zero. The rotor must always lag (slip) slightly behind the synchronous speed. For example, a standard 4-pole motor with an $N_s$ of 1800 RPM will typically spin an actual full-load speed of around 1750 ROM.

### Reverse

Reversing any two phases changes the electrical time sequence of the currents relative to the spatial sequence of the windings. The inversion fips the mathematical direction of the vertical component, causing the net magnetic field to rotate backward.

In a standard three-phase system, the currents peak in a specific forward time order: A -> B -> C. Beacuse the stator windings are physically arranged clockwise around the frame in the same order, the peak magnetic pull naturally sweeps clockwise. Swapping the wires of Phase B and Phase C without changing their physical position on the stator frame changes the time order to: A -> C -> B, spinning the magnetica field in the opposite direction.

## Slip and Rotor Behavior

### Slip

Slip is the velocity lag of the rotor behind the spinning magnetic field or the normalized difference between synchronous speed and the rotor speed. The motor cannot run at synchronous because doing so means there would be no relative motion between the rotor and magnetic field, nothing would cut through the magnetic field, and no induced EMF or torque would be generated. Naturally, if an induction motor were to reach synchronous speed (receiving a downhill load, for example), the drop in torque would immediately lower the rotor's rotational velocity and restore current and torque as slip appears.

Slip is calculated as a dimensionless ratio or percentage using:

$$s = {N_s - N_r \over N_s}$$

where:

$s$: Slip is expressed as a decimal or multiplied by 100 for a percentage

$N_s$: Synchronous speed of the stator magnetic field (RPM)

$N_r$: Actual rotational speed of the physical rotor (RPM)

For standard industrial induction motors, typical full-load slip values range strictly between 1% and 5%. This narrow range indicated highly efficient operation. It's not a fixed value and scales dynamically with the physical torque demanded by the mechanical load. No-load conditions would have slip values at 0.5% or lower as the rotor spins nearly synchronous with the field. Increasing the load creates mechanical drag that slows the rotor down and increases slip. Higher slip increases relative motion, inducing more current to match the load. If slip approaches 15-20%, the motor reaches its maximum breakdown torque and will stall.

![Induction Motor Slip Graph](inductionMotorSlipGraph.png)

### Rotor Frequency

The relationship between the stator's electrical supply frequency ($f_s$) and the resulting frequency of the current induced in the rotor bars ($f_r$) is governed directly by the motor's slip (s):

$$f_r = s \times f_s$$

The frequency of the induced voltage and current in the rotor is determined strictly by relative speed. It depends entirely on how fast the stator's magnetic field is sweeping past the physical rotor bars.

At startup ($N_r = 0$), the rotor is completely stationary, so the stator field passes the rotor bars at full synchronous speed ($N_s$). Slip is 100%, the rotor frequency matches the line frequency exactly ($f_r = 100\% \times 60 \text{Hz}$), and the motor behaves exactly like a short-circuited transformer, inducing high-frequency, high-magnitude currents. 

At synchronous speed (theoretical $N_r = N_s$), slip would be zero, the rotor frequency would drop to zero, zero current is induced, and torque would vanish.

For a real-world motor operating at 3% slip in North America (60Hz supply):

$$f_s = 60 \text{Hz}$$

$$\text{Slip}(s) = 0.03$$

$$f_r = s \times f_s$$

$$f_r = 0.03 \times 60 \text{Hz} = 1.8 \text{Hz}$$

A frequency of 1.8Hz means the alternating current insides the coper rotor bars changes direction only 1.8 times per second. This is so slow that it mimics a slowly fluctuating direct current (near-DC). 

The drop in frequency from 60Hz at startup down to 1-2Hz at full loard drastically alters the internal electrical behavior of the rotor:

- The electrical opposition to current flow in the rotor comes from two sources: resistance and inductive reactance. Inductive reactance depends heavily on frequency ($X_r = 2\pi f_rL_r$). At startup, inductive reactance, $X_r$, is high, shich chokes the current and pushes it out of phase with the stator field, resulting in a poor startup power factor. At full load (1-2Hz), $X_r$ plumments to nearly zero. The rotor becomes almost entirely resistive.

- Because the near-DC frequency eliminates the inductive lag, the induced rotor current peaks at the exact same physical location as the maximum stator magnetic flus. The alignment is the reason why the motor produces maximum running efficiency and stable torque at low slip values despite the low frequency.

- Magnetic hysteresis [[^3]](#footnote-3) and eddy current losses inside the steel core of the rotor are heavily frequency-dependent. Because the rotor current drops to near-DC during normal operation, iron losses in the rotor become so tiny they are usually ignored in standard engineering efficiency calculations. 

[^3]: Magnetic hysteresis is the lag between an applied external magnetic field and the resulting magnetization of a ferromagnetic material. When iron or steel is magnetized and demagnetized, the internal magnetic domains do not instantly reset to zero when the outside force stops.

## Torque-Speed Characteristics

The torque-speed curve of an induction motor details the mechanical torque the motor can develop as it accelerates froma a complete standstill up to its synchronous speed. Because an induction motor behaves like a varying inductive loads as its slip changes, its torque output does not follow a straight line. It forms a distrinct, non-linear curve characterized by four primary operating points and regions.

1. Starting Torque (Locked-Rotor): Starting torque is the mechanical turning force developed by the motor the exact instant power is applied to the stator windings while the rotor is at a complete standstill (0 RPM, Slip = 100%).

    - The rotor frequency($f_r$) is at its maximum. This causes high inductive reactance ($X_r$) in the rotor bars.

    - Although a massive inrush current flows into the motor at startup (typically 6-8 times the normal running current), the inductive lag creates a poor power factor. The magnetic field of the rotor is physically misaligned eith the stator's field, resulting in a starting torque of 100% to 200% of the motor's rated full-load torque.

2. As the motor begins to rotate and accelerate, the speed increases, and the slip drops below 100%. **Pull-up torque** is the absolute lowest point of torque on the curve during acceleration.

    - For many standard motors, as the rotor begins to accelerate, parasitic synchronous torques (caused by spatial harmonics or the physical alignment of the stator and rotor slot configurations) create a localized counter-torque.

    - The motor must produce enough torque at this dip to exceed the static torque required by the conneted mechanical load. If the load's torque requirement is higher than the motor's pull-up torque, the motor will hand at this intermediate speed, fail to accelerate further, draw destructive inrush currents, and eventually trip its thermal overloads.

3. As the motor clears the pull-up region, it enters a phase of rapid acceleration. **Breakdown torque** is the absolute maximum torque the motor can physically produce.

    - This peak occurs at the exact physical speed where the dropping inductive reactance of the rotor ($X_r$) equals the internal electrical resistance of the rotor ($R_r$). Mathematically, this is the point of maximum power transfer to the rotor.

    - Breakdown torque is usually high (ranging from 200% to 350% of the motor's rated full-load torque). It represents the motor's ultimate capacity to handle sudden, short-term mechanical overloads without stalling.

4. Once the motor passes its breakdown peak, it drops down into its normal, highly efficient working zone. 

    - Within this narrow operating range, slip is small (1% to 5%) and the rotor frequency has dropped to its optimized near-DC state. The torque-speed relationship behaves like a straight line.

    - This is the **stable region** because the motor naturally self-regulates against changes in the mechanical load. Increasing a load increases slip, which increases relative motion, which induces higher voltage and surge in near-DC current, which outputs exactly enough additional torque to balance the heavier load

If a mechanical jam forces the load torque to exceed the breakdown torque, the motor falls out of this linear zone. It enter the "unstable region," where slowing down decreases the available torque, leading to an immediate mechanical stall.

### Torque Equation

The simplified torque equation for a three-phase induction motor is a foundational engineering formula. It models how changing rotor speed changes the motr's mechanical torque output:

$$T \propto {s \cdot R_2 \over R_2^2 + (s \cdot X_2)^2}$$

(note: in classical machine design notation, the subscript "2" denotes the rotor parameters, replacing the general "r" subscript).

This equation is derived directly from the priciple of maximum power transfer applied to an inductive circuit. 

- The numerator $(s \cdot R_2)$:  
    The numerator dictates the baseline capacity of the motor to generate an induced current that can align properly with the stator's magnetic field.

    - $s$ (Slip): When the motor is running at no-load, slip is nearly zero. very little or no voltage is induced and torque, likewise, is very little or zero. As a load forces the rotor to slow down, slip increases linearly, drawing more power from the stator by inducing a higher voltage in the rotor bars.

    - $R_2$ (Rotor Resistance): The physical resistance of the copper or aluminum bars embedded in the rotor core. It governs the active, in-phase current that produces the real mechanical work. In the numerator, a higher $R_2$ directly increases the initial torque output of the motor at high slip values (like startup).

- The denominator ($R_2^2 + (s \cdot X_2)^2$):  
    The denominator represents the square of the total rotor impedance ($Z_2^2$). It acts as an electrical bottleneck that limits how much current can physically flow through the rotor bars.

    - $R_2^2$ (Rotor resistance squared): This acts as the steady, unchanging lower limit of rotor impedance. Because the physical copper bars do not change shape while spinning, this term remains constant regardless of the motor's speed.

    - $(s \cdot X_2)^2$ (Variable inductive reactance squared): This is the most dynamic part of the equation. $X_2$ is the standstill inductive reactance (the rotor's internal inductance at 100% grid frequency). As the motor spins, the rotor frequency drops ($f_r = s \cdot f_s$). Because inductive reactance is frequency-dependent ($X_L = 2\pi f L$), it must be scaled by the slip.

### Low Slip vs. High Slip Regions

The behavior of the denominator explains why the torque-speed curve has its unique, curved shape, splitting normal motor operation into two distinct physical zones.

**Low Slip (stable) Region**

When the motor is running normally under its design load, the slip is tiny (1% to 5%). Because slip is so small, the term $(s \cdot X_2)^2$ becomes negligibly small compared to $R_2^2$ and the equation can be simplified to $T \propto {s \cdot R_2 \over R_2^2} \Rightarrow T \propto {s \over R_2}$. Physically at running speeds, the rotor current frequency is near-DC, meaning inductive lag completely vanishes. The rotor behaves like a pure resistor. In this zone, torque is directly proportional to slip. If the load increases, the motor slows down slightly, and torque increases in a predictably to match the load.

**High Slip (Unstable/Startup) Region**

When the motor first starts up or is heavily overloaded, the slip is large (50% to 100%). At high slip, the rotor frequency is high. The inductive reactance term $(s\cdot X_2)^2$ grows so massive that it completely dominates the fixed resistance ($R_2^2$). Mathematically, $R_2^2$ can be dropped from the denominator and simplify the equation to $T \propto {s\cdot R_2 \over(s\cdot X_2)^2} \Rightarrow T \propto {R_2\over s\cdot X_2^2}$. Physically, the rotor is highly inductive. Even though a massive current flows into the rotor bars, it lags severely behind the voltage. The maximum rotor magnetic poles do not line up with the maximum stator magnetic poles. In this zone, torque is inversely proportional to slip. This explains why starting torque is significantly lower than peak torque and why slowing down past the breakdown point causes the motor to stall.

**The Breakdown Peak ($R_2 = s\cdot X_2$)

The peak of the torque-speed curve (Breakdown Torque) occurs at the exact mathematical tipping point where the two terms in the denominator balance perfectly: $R_2 = s \cdot X_2$

Physically, this is the exact speed where the rotor frequency has dropped low enough that its inductive lag no longer chokes the current, allowing the maximum possible transfer of electromagnetic power from the stator into the rotor. 

When the mechanical load torque exceeds the motor's breakdown torque, the motor falls out of its self-regulating balance and enters sequence that leads to a mechanical stall:

1. A sudden drop into the unstable region

    Normally, an induction motor operates in its stable region where a slight decrease in speed causes a necessary increase in torque to match the load. The moment the load torque surpasses the peak breakdown torque point ($R_2 = s\cdot X_2$),  the motor crosses a dangerous thermodynamic boundary into the unstable region where slowing down causes the motor's torque output to decrease.

2. Stall

    Because the load is demanding more pulling force than the motor can physically generate, a rapid chain reaction occurs.

    1. Deceleration: The heavy load acts as a severe brake, forcing the physical rotor speed ($N_r$) to plummet towards 0 RPM.

    2. Slip skyrockets: As the rotor slows down, the slip rapidly climbs towards 100%.

    3. Inductive reactance dominates: As slip climbs, the frequency of the current inside the rotor bars ($f_r = s\cdot f_s$) spikes from its normal 1-2Hz up toward the full grid frequency.

    4. Torque collapse: According to the torque equation ($T\propto {R_2 \over s\cdot X_2^2}$), the massive spike in frequency cause sthe rotor's inductive reactance ($s\cdot X_2$) to balloon. This chokes off the active current and throws the rotor's magnetic field completely out of phase with the stator field. Torque plummets and the motor locks up at 0 RPM.

3. Locked-Rotor Current

    Once the motor has stalled, it is in a "locked-rotor" state. Because there is no longer any counter-electromotive force being generated by the rotor's rotation to oppose the incoming voltage, the motor effectively becomes a massive short circuited transformer.

    The stator instantly begins drawing locked-rotor amperage (6 to 8 times higher than its normal full-load running current). A motor that normally draws 20 Amps under full load will suddenly pull 120 to 160 Amps while sitting still and humming loudly.

4. Thermal Destruction

    Electrical copper losses ($I^2R$) heat up a motor. Because the current has increased six-fold, the internal heat generation spikes by a factor of thirty-six ($i^2\Rightarrow6^2=36$). Because the rotor has stopped spinning, the internal shaft-mounted cooling fan is no longer spinning. The motor has zero airflow to dissipate the thermal surge. The temperature inside the stator slots rises to a level where the thin varnish insulation coating the copper windings melts or bakes dry and cracks.

To keep the motor from catching fire or melting its windings when a stall occurs, modern industrial electronical systems rely on overload relays/thermal magnetic breakers that sense the LRA current spike. If current does not drop back down to normal running levels within a predefined window, the breaker trips the contactor and cuts line power to the motor. 

If the motor is controlled by a variable frequency drive, the drive will actively monitor the slip. If it sees the torque demand hitting the breakdown threshold, it will automatically lower the stator frequency to decrease the slip, safely dropping the motor speed while maintaining maximum torque to pull through the jam without tripping.

### Reducing Stator Voltage

Reducing the voltage cupplied to the stator shifts the entire torque-speed curve downward because of a strict mathematical and physical rule: indection motor torque is directly proportional to the square of the stator voltage ($T\propto V_s^2$). This relationship is why simple coltage reduction is terrible for speed control under heavy loads and why Volts-per-Hertz ($V/f$) control is used in variable frequency drives instead.

The fundamental motor torqu equation can be explanded to show its dependance on the stator supply coltage ($V_s$):

$$T \propto {s \cdot R_2 \cdot V_s^2 \over R_2 ^ 2 + (s \cdot X_2)^2}$$

- Stator voltage directly creates the stator's rotation magnetic flux ($\Phi_s \propto V_s$). This flux must then physically corss the air gap to induce voltage and subsequent current inside the rotor bars. The rotor current is directly proportional to the stator voltage ($I_2 \propto V_2$).

- Because torque is the product of stator flux and rotor current ($T \propto \Phi_s \cdot I_2$), any drop in voltage cuts both terms.

- Reducing the stator voltage by half, for example, drops the maximum available breakdown torque by a factor of four. It reduces to 25% of its original capacity. The stable operating region is shrunk and chances of stall increase.

To change the speed of an induction motor safely and efficiently, the frequency of the electricity is altered because synchronous speed is directly tied to frequency ($N_S = {120f \over P}$).

**The danger of dropping frequency alone**

If a VFD lowers the frequency from 60Hz to 30Hz to slow a conveyor belt to half speed, but keeps the voltage at 460V, the $V/f$ ratio doubles. Physically, this means the alternating current is changing direction so slowly that the magnetic field has twice as much time to build up in the steel stator teeth during each electrical cycle. This pushes the steel core deep into magnetic saturation. The core can no longer contain the lines of flux, inductive reactiance pummets to near zero, and the motor draws a destructive wave of magnetizing current, melting the windings.

To prevent core saturation while still allowing full speed control, a VFD uses a variable voltage variable frequency (VVVF) inverter. When the drive changes the frequency, it automatically shifts the voltage by the exact same proportion to keep the $v/f$ ratio constant.

By lockign the $V/f$ ratio at a constant value, the VFD ensures that the magnetic flux inside the motor remains perfectly steady at its ideal, full design level across the entire speed spectrum. Because the magnetic flux is kept constant, the motor can produces its maximum rated breakdown torque at any speed, from a crawl up to full velocity. Instead of collapsing the torque curve downward, the VFD smoothly slides the enitre full-strength torque curve horizontally, giving high-torque speed control without any risk of stalling or overheating.