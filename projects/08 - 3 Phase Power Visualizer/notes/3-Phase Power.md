# 3-Phase Power

Three-phase power is an electrical system that uses three separate alternating currents transmitted over three wires. Each current wave is offset by 120 electrical degrees that provides a continuous, uninterrupted flow of energy. Commercial and industrial facilities usually use a three-phase supply to better accommodate higher loads as compared to the single-phase power supply that residential homes are usually served. A three-phase power supply can transmit three times as much power as a single-phase power supply while only needing one additional wire (three instead of two).

## Problems with Single-Phase Power

Single-phase electricity relies on a single alternating current waveform that rises and falls periodically. The voltage waveform crosses the zero-volt line twice with each AC cycle. The power output drops to zero 120 times per second on a 60Hz grid (or 100 times per second on a 50Hz grid in Europe). Electrical motors running on single-phase supplies experience a jerky torque output due to the periodic power supply. The constant pulsing creates excessive mechanical vibration, louder operation, and faster wear on internal components. Large single-phase motors are also incapable of self-starting. They require complex additional components like start-capacitors, centrifugal switches, or shading coils to kickstart rotation.

## How Three-Phase Power Solves these Problems

Three-phase systems combine three separate AC voltage wave forms. With each wave delayed or offset by exactly $120^\circ$ relative to the next. Because the three wave forms are staggered, one phase is always peaking or climbing as another falls. The mathematical sum of the power from all three phases remains completely constant at every single instant. The jerky torque output is no longer present and, therefore, eliminates the loud mechanical vibrations and faster wear on internal components. The $120^\circ$ physical separation of the currents naturally generates a rotating magnetic field inside the electrical motor that self-starts the motor as soon as power is delivered without the need for the previously mentioned additional components.

## 3-Phase Uses Less Conductive Material

A standard single-phase system must have two wires of equal thickness: one hot wire to carry the current to the load and one neutral wire to return that exact same current to the source. In a balanced three-phase system (specifically a Wye or Delta configuration[[^1]](#footnote-1)), the currents are out of phase by $120^\circ$. The sum of which becomes zero and a neutral wire returning to the power plant is no longer necessary. 

$$\sin(\theta) + \sin(\theta - 120^\circ) + \sin(\theta + 120^\circ) = 0$$

[^1]: Wye and Delta are the two fundamental methods for wiring three-phase alternating power systems. Wye (Star/Y) and Delta (Triangle/$\Delta$) define how the three live phase windings or loads connect to each other to generate,distribute, or consume electrical energy. A Wye configuration provides two different voltages. In a stardard and conveniently named 120/208V system, 208V is provided between any two phases and 120V between any single phase and neutral. Delta configuration provides a single uniform line-to-line voltage across all phases (standard 240V or 480V).

Because the power is spread out across three separate wires, three-phase systems have higher capacities and use thinner wires than an equivalent single-phase system. In commercial power use that can easily use 30kW or more, a single single-phase system would require impractically thick wires that suffer massive heat generation that wastes energy and presents a severe fire hazard. Using multiple single-phase systems massively increases material costs with each system requiring two additional wires and increases the complexity of transformers, generators and switchgear 

## The Three Balanced Voltages

The three balanced voltages of a three-phase system are creatively called Phase A, Phase B, and Phase C. They share identical peak amplitude ($V_m$) and frequency, but use successive $120^\circ$ phase offsets in time and phasor domains to create smooth, continuous power. The equations for each are:

### Frequencies

$\omega = 2 \pi f$

Where $f$ is the system frequency (Usually 50Hz or 60Hz).

### Time-Domain

Phase A ($0^\circ$ offset): $v_a(t) = V_m \cos (\omega t)$

Phase B ($-120^\circ$ offset): $v_b(t) = V_m \cos (\omega t - 120^\circ)$

Phase C ($-240^\circ$ offset): $v_c(t) = V_m \cos (\omega t - 240^\circ)$

### Phasor Form

Phase A phasor: $V_a = V_m \angle 0^\circ$

Phase B phasor: $V_b = V_m \angle -120^\circ$

Phase C phasor: $V_c = V_m \angle -240^\circ$

## Unbalanced Voltages

Unbalanced voltages happen when the amplitudes of the three phases differ or their $120^\circ$ phase angles shift. 

### Mathematically

When a system becomes unbalanced, it is mathematically broken down into three symmetrical component systems using Fortescue's Theorem. [[^2]](#footnote-2):

[^2]: Any unbalanced set of N polyphase phasors can be broken down into N symmetrical sets of balanced phasors. It is used for three-phase systems to resolve unbalanced voltages and currents into: positive-sequence, negative sequence, and zero-sequence components.

- Positive-sequence components: Three equal vectors shifted by $120^\circ$ rotating in normal order (A→B→C)

- Negative-sequence components: Three equal vectors shifted by $120^\circ$ rotating in reverse order (A→C→B)

- Zero-sequence components: Three identical vectors with the same magnitude and no phase shift($0^\circ$)

### Physical Effects

The phase currects no longer sum to zero. The current no longer sums up to zero and the leftover current flows into a dedicated neutral wire back to the source. Negative-sequence voltages create a counter-rotating magnetic field in motors that act like a brake, producing counter-torque, mechanical vibration, and destructive winding heat. Increased $I^2R$ power losses occur across transmission lines and distribution networks.

### Common Causes

- Unequal load distribution: connecting too many single-phase loads (homes or offices) to one specific phase.

- Fault conditions: Line-to-ground or line-to-line faults that temporarily drag down the voltage of an individual phase.

- Blown fuses: A blown fuse on a single phase creates an extreme single-phasing condition for three-phase equipment.

### Unplanned Unbalanced Loads

In a Wye configuration, a neutral wire can be installed as a safety valve in the event of unplanned failures that cause unbalanced loads. The neutral wire can safely carry the current back to the source while the two remaining wires can operate at normal voltages to protect single-phase equipment. Without a neutral wire, the current in one can drop while the other spikes dangerously high. Electronics connected to the line will be instantly destroyed.

In a Delta configuration, a neutral wire cannot physically have a neutral wire. In the event of something like a blown fuse, electrical equipment experiences single-phasing where the two remaining phases pull massive amount of current to keep up. Because of the lack of a neutral wire, protective relays and thermal overloads are installed to detect current loss or phase spikes and trip a breaker before machinery destroys itself.

### Phase Sequence

A-B-C positive phase sequence means voltage peaks arrive in the order of A, then B, then C spaced $120^\circ$ apart. Reversing any two power wires changes the order to A-C-B negative sequence. This flips the physical rotation of the motor's internal magnetic field, forcing the rotor to spin backwards.

Inside motors, three physical coils are placed around the frame, connected to wires A, B, C. In positive phase sequence, voltage peaks arrive in the order of A, B, and C. As the power waves peak one after another, they create a magnetic pull that moves in a circle. The metallic rotor chases the moving magnetic circle.

Swapping the wires to negative sequences changes the peak order to A-C-B, makes the magnetic field spin the other way, and reverses the spin of the rotor.

## Star and Delta Configurations

Star and delta configurations are the two ways to connect three-phase electrical systems. A star (Y) conection links each phase to a central neutral point, making line voltage $\sqrt{3}$ times the phase voltage while line current equals phase current. A delta ($\Delta$) connection forms a closed loop, making line voltage equal to phase voltage while line current is $\sqrt{3}$ times the phase current.

               L1                                  L1
               \                                   *
               /  Winding 1                       / \
               \                                 /   \
               |                          Winding     Winding
               +----- N (Neutral)            1           3
              / \                              /       \
             /   \                            \         /
        Winding   Winding                    /           \
          2        3                        *-------------*
         /           \                     /               \
        /             \                   L2               L3
        L2           L3               Winding 2
             STAR                                DELTA

### Star (Y) Configuration

Three phase windings connect to a single central point called the neutral point. It uses four wires (three live phases and one neutral). 

The line voltage ($V_L$) is $\sqrt{3}$ times the phase voltage ($V_{ph}$):

$$V_L = \sqrt 3 \times V_{ph}$$

The lines current ($I_L$) is equal to the phase current ($I_{ph}$):

$$I_L = I_{ph}$$

**Key features** 

The Star configuration can supply two different voltages obtained from line-to-line and line-to-neutral connections. Typical 120/208V configuations give 208V when connecting any two phases and 120V when connecting one phase to neutral.

Unbalanced loads are handled safely through the neutral wire.

### Delta ($\Delta$) Configuration

Three phase windings connect end-to-end in a triangle loop with no central neutral point. It uses three wires (three live phases).

The line voltage ($V_L$) equals the phase voltage ($V_{ph}$):

$$V_L = V_{ph}$$

The line current ($I_L$) is $\sqrt 3$ times the phase current ($I_{ph}$):

$$I_L = \sqrt 3 \times I_{ph}$$

**Key Feature**

The delta configuration is robust and self-contained. If one phase fails, the other two can still supply power in an open-delta arrangement (at reduced total capacity).

### Applications 

**Star Configuration**

- Long-distance power transmission lines. Lower phase voltage requires less insulation.

- Commercial and residential low-voltage distribution (providing both 400V/230V or 208V/120V).

- Soft-starting large electic motors to reduce high initial inrush current.

**Delta Configuration**

- Heavy industrial equipment and large motors requiring high starting torque.

- Power distribution networks over short distances.

- The primary side of substations and transformers feeding balanced loads.

### The $\sqrt3$ factor

The $\sqrt3$ factor is derived from the phasor subtraction of two phase voltages that are separated by a $120^\circ$ angle. 

In a balanced Star connection, line-to-line ($V_L$) is measured between two different phase terminals. The line voltage is the vector difference between the two phase voltages relative to the neutral point($N$).

$$\vec{V}_{AB} = \vec{V}_{AN} - \vec{V}_{BN}$$

Assuming the RMS magnitude of each phase voltage is $V_{ph}$, the two vectors are separated by exactly $120^\circ$. Subtracting $\vec{V}_{BN}$ inverts its direction (creating a $60^\circ$ angle relative to $\vec{V}_{AN}$).

Using the law of cosines for vector subtraction (where the angle between the two original vectors is $\theta = 120^\circ$):

$$V^2_L = V^2_{ph} + V^2_{ph} - 2 \cdot V_{ph} \cdot V_{ph} \cdot \cos(120^\circ)$$

$$V^2_L = V^2_{ph} + V^2_{ph} - 2 \cdot V^2_{ph}(-0.5)$$

$$V^2_L = V^2_{ph} + V^2_{ph} - 2 V^2_{ph}(-0.5)$$

$$V^2_L = V^2_{ph} + V^2_{ph} + V^2_{ph}$$

$$V^2_L = 3 V^2_{ph}$$

$$V_L = \sqrt3 \cdot V_{ph}$$

### Star and Delta configurations are duals[[^3]](#footnote-3) of each other

[^3]: In network theory, duality means that the equations governing one circuit configuration mirror the equations of another if you swap specific variables (Voltage <-> Current, Series <-> Parallel).

**Star/Delta Duality Table**

| Attribute | Star Configuration | Delta Configuration |
| --- | --- | --- |
| Circuit Type | Series-like node connections | Parallel-like loop connections |
| Governing Law | Kirchhoff's Voltage Law | Kirchhoff's Current Law |
| Equal Variable | Line Current = Phase Current ($I_L = I_{ph}$) | Line Voltage = Phase Voltage ($V_L = V_{ph}$) |
| $\sqrt{3}$ Variable | Line Voltage = $\sqrt3 \times V_{ph}$ | Line current = $\sqrt3 \times I_{ph}$ |

In Star (KVL Dual): Two phase voltage vectors meet at a central serial node. To find the line voltage, KVL is used to find the difference between two potentials separated by $120^\circ$. The vector math yields $\sqrt3 \cdot V_{ph}$.

In Delta (KCL Dual): Two phase current vectors meet at a parallel line junction. To find the line current, KCL is used to find the difference between two entering/exiting currents separated by $120^\circ$ ($\vec{I}_L = \vec{I}_{phaseA} - \vec{I}_{phaseB}$).

Because of the geometric angles and vector subtraction mathematics are identical, the current in a delta configuration scales by the same $\sqrt3$ factor as the voltage does in a star configuration.

## 3-Phase Power Equation

The total three-phase power formula is:

$$P = \sqrt 3 \cdot V_L \cdot I_L \cdot \cos (\phi)$$

Where:

* $P$ is the total active power in Watts (W).

* $V_L$ is the line-to-line RMS voltage.

* $I_L$ is the line RMS current.

* $\cos (\phi)$ is the system power factor (the phase angle difference between voltage and current).

Alternatively, using phase variables, the total power is:

$$P = 3 \cdot V_{ph} \cdot I_{ph} \cdot \cos (\phi)$$

In a balanced system where the power factor is 1.0 ($\phi = 0^\circ$), the voltages for the three phases (shifted by $120^\circ$ or ${2 \pi} \over 3$ radians) are:

$$v_1(t) = V_m \sin (\omega t)$$

$$v_2(t) = V_m \sin (\omega t - {{2 \pi} \over 3})$$

$$v_3(t) = V_m \sin (\omega t - {{4 \pi} \over 3})$$

The currents match the voltage profiles:

$$i_1(t) = I_m \sin (\omega t)$$

$$i_2(t) = I_m \sin (\omega t - {{2 \pi} \over 3})$$

$$i_3(t) = I_m \sin (\omega t - {{4 \pi} \over 3})$$

Instantaneous power in each phase is $p=v(t) \cdot i(t)$. Using the trigonometric identity $\sin ^2(\theta) = {{1 - \cos(2 \theta)} \over 2}$, the power equations become:

$$p_1(t) = V_mI_m\sin^2(\omega t) = {{V_mI_m} \over 2}[1 - \cos(2\omega t)]$$

$$p_2(t) = V_mI_m\sin^2(\omega t - {2\pi\over3}) = {V_mI_m\over2}[1 - \cos(2\omega t-{4\pi\over3})]$$

$$p_3(t) = V_mI_m\sin^2(\omega t-{4\pi\over3}) = {V_mI_m\over2}[1 - \cos(2\omega t-{8\pi\over3})]$$

The total instantaneius power, $p_{total}(t) = p_1(t) + p_2(t) + p_3(t)$ is:

$$p_{total} = {V_mI_m\over2}[3-(\cos(2\omega t)+\cos(2\omega t-{4\pi\over3})+\cos(2\omega t-{8\pi\over3}))]$$

The three sosine terms represent a balanced thre0phase set of waves aoscillating at twice the system frequency ($2\omega$). The sum of any balanced three-phase sinusoidal set is always zero:

$$\cos(2\omega t) +\cos(2\omega t-{4\pi\over3}) + \cos(2\omega t-{8\pi\over3}) = 0$$

Substituting zero back into the equation removes the time-dependent variable completely:

$$p_{total}(t) = {3\over2} V_mI_m$$

Because the maximum values relate to RMS values by $v_m = \sqrt 2 V_{ph}$ and $I_m=\sqrt{2} \cdot I_{ph}$, the equation simplifies into a flat constant:

$$p_{total}(t)=3\cdot V_{ph}\cdot I_{ph}$$

### Why it matters

Single-phase motors experience structural vibration because their power drops to zero twice every cycle. Three-phase motors receive smooth and continous torque that vastly extends the lifespan of heavy industrial equipment. Generators and transmission lines also operate at maximum efficiency because they do not have to absorb or buffer cyclical energy pulses.

# Usage: AC Motors

A rotating magnetic field (RMF) is created by feeding three-phase AC current into three stator windings that are physically spaced $120^\circ$ apart. This geometry combined with AC current shifts causes the net magnetic field vector to maintain a constant strength while seamlessly rotating $360^\circ$ in space.

Each individual winding around the stator generates a stationary, pulsating magnetic field along its physical axis. When the individual fields are summed using vectors, a unique phenomenon occurs:

- At any single point in time, the total magnetic field strength equals exactly 1.5 times the maximum field strength of a single coil.

- As the AC currents change value over time, the combined total vecto does not grow or shrink. Its direction smoothly rotates around the stator.

- One full cycle of the alternating current causes the magnetic field vector to complete exactly one full physical rotation in a 2-pole motor. The rotational rate is called the synchronous speed.

### Rotation Cycle

Consider a standard 2-pole motor layout as time advances through one AC electrical cycle:

- At $0^\circ$: Current in Phase A is at its positive maximum. Phases B and C are negative and half-strength. The net magnetic vector points directly along the Phase A axis.

- At $120^\circ$: Current in Phase B reaches its positive maximum. Phase A and C drop to negative half-strength. The net magnectic vector has a now rotated $120^\circ$ in space to line up with Phase B.

- At $240^\circ$: Current in Phase C peaks positively. The combined magnetic vector has now rotated $240^\circ$ in space to line up with Phase C.

- At $360^\circ$: The currents return to their initial state and the net magnetic field vector completes its $360^\circ$, starting the cycle over.

### Speed control

Speed is governed by a variable frequency drive (VFD) (also known as an adjustable-speed drive or inverter).

The synchronous speed of the magnetic field depends entirely on the AC frequency ($N_s = {120f \over P}$), the speed can not change by lowering the voltage. The frequency of the electricity going into the motor must change.

A VFD changes the incoming utility power (which sits at a fixed 50Hz or 60Hz) into an adjustable frequency using a three-step internal process:

- The rectifier: Diode bridges take the incoming fixed AC voltage and convert it into direct current power.

- The DC Bus: Capacitors filter and smooth out the ripple from the converted DC power to store it cleanly.

- The Inverter: High-speed electronic switches turn the DC power back into AC.

The inverter does not output a smooth and continuous sine wave. It uses Pulse Width Modulation to simulate a synthetic AC sine wave at any target frequency. If the VFD outputs 30Hz instead of 60Hz, the stator's magnetic field will rotate at exactly half its original speed.

When the VFD lowers the frequency, it also lowers the voltage. Lowering the frequency while keeping the voltage high will cause the stator windings to draw too much current and overheat. The VFD maintains a constant Volts-per-Hertz ratio to ensure the motor maintains its optimal magnetic field strength without burning out.

### Slip

An induction motor rotor must always run slower than the rotating magnetic field because zero speed difference means zero induced current, zero magnetic field in the rotor, and zero torque. This difference in speed is called slip. It's the physical mechanism that allows the motor to pull a mechanical load.

Slip ($s$) is the relative difference between the synchronus speed of the stator's magnetic field ($N_s$) and the actual mechanical speed of the rotor ($N_r$). It is expressed as a percentage:

$$s = {N_s - N_r \over N_s} \times 100\%$$

At standstill: The rotor is locked ($N_r = 0$), so slip is 100%. Under normal load, the motor runs efficiently with a typical slip between 1%-5%.

Induction motors are asynchronous machines. They rely entirely on electromagnetic induction rather than direct electrical connections or permanent magnets to turn the rotor.

        [Stator Field Spins at Ns] 
                   │
                   ▼ (Relative speed difference = Slip)
        [Cuts Rotor Bars] 
                   │
                   ▼ (Faraday's Law)
        [Induces Voltage & Current in Rotor] 
                   │
                   ▼ (Creates Rotor Magnetic Field)
        [Interaction: Stator Field pulls Rotor Field] ──► [MECHANICAL TORQUE]

- Faraday's Law of Induction [[^4]](#footnote-4): To induce a voltage in the rotor bars, the bars must physically cut through the magnetic flux lines of the stator's rotating field.

[^4]: A voltage (known as an electromotive force (EMF)) is induced in a circuit whenever there is a changing magnetic flux. Think of a magnet moving through a coil.

- Magnetic flux lines are only cut if there is a difference in speed between the spinning stator field and the moving rotor bars.

- The cutting action induces an electromotive force (voltage), which drives a high current through the short- circuited rotor bars. This current creates the rotor's own magnetic field.

- The stator's magnetic field interacts with the rotor's induced magnetic field, creating the Lorentz force[[^5]](#footnote-5) that drags the rotor around.

[^5]: The Lorentz force is the total push or pull given to a tiny charged piece of matter by electrical fields and magnetic fields.

If the rotor were to accelerate and hit 100% synchronous speed, the motor would instantly lose its ability to turn. The rotor bars would travel at the exact same speed as the stator's magnetic field, would ride along with the magnetic wave rather than cutting through it, induced voltage drops to exactly zero (current stops flowing in the rotor), and - with no rotor current - the rotor loses its magnetic field, dropping torque to zero.

Because of friction, wind resistance, and mechanical loads, a rotor with zero torque will instantly slow down, causing slip to return, current to flow again, and restore torque.