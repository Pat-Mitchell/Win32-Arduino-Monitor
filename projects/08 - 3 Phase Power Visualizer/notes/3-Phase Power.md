# 3-Phase Power

Three-phase power is an electrical system that uses three separate alternating currents transmitted over three wires. Each current wave is offset by 120 electical degrees that provides a continuous, uninterrupted flow of energy. Commercial and industrial facilities usually use a three-phase supply to better accommodate higher loads as compared to the single-phase power supply that residential homes are usually served. A three-phase power supplu can transmit three times as much power as a single-phase power supply while only needing one additional wire (three instead of two).

## Problems with Single-Phase Power

Single-phase electricity relies on a single alternating current waveform that rises and falls periodically. The voltage waveform crosses the zero-volt line twice with each AC cycle. The power output drops to zero 120 times per second on a 60Hz grid (or 100 times per second on a 50Hz grid in Europe). Electrical motors running on single-phase supplies experience a jerky torque output due to the periodic power supply. The constant pulsing creates excessive mechanical vibration, louder operation, and faster wear on internal components. Large single-phase motors are also incapable of self-starting. They require complex additional components like start-capacitors, centrifugal switches, or shading coils to kickstart rotation.

## How Three-Phase Power Solves these Problems

Three-phase systems combine three separate AC voltage wave forms. With each wave delayed or offset by exactly $120^\circ$ relative to the next. Because the three wave forms are staggered, one phase is always peking or climing as another falls. The mathematical sum of hte power from all three phases remains completely constant at every single instant. The jerky torque output is no longer present and, therefore, eliminates the loud mechanical vibrations and faster wear on internal components. The $120^\circ$ physical separationg of the currents naturally generates a rotating magnetic field inside the electrical motor that self-starts the motor as soon as power is delivered without the need for the previously mentioned additional components.

## 3-Phase Uses Less Conductive Material

A standard single-phase system must have two wires of equal thickness: one hot wire to carry the current to the load and one neutral wire to return that exact same current to the source. In a balanced three-phase system (specifically a Wye or Delta configuration[[^1]](#footnote-1)), the currents are out of phase by $120^\circ$. The sum of which becomes zero and a neutral wire returning to the power plant is no longer necessary. 

$$\sin(\theta) + \sin(\theta - 120^\circ) + \sin(\theta + 120^\circ) = 0$$

[^1]: Wye and Delta are the two fundamental methods for wiring three-phase alternating power systems. Wye (Star/Y) and Delta (Triangle/$\Delta$) define how the three live phase windings or loads connect to each other to generate,distribute, or consume electrical energy. A Wye configuration provides two different voltages. In a stardard and conveinently named 120/208V system, 208V is provided between any two phases and 120V between any single phase and neutral. Delta configuration provides a single uniform line-to-line voltage across all phases (standard 240V or 480V).

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

Phase C phasor: $V_c = V_m \angle -240^circ$

## Unbalanced Voltages

Unbalanced coltages happen when the amplitudes of the three phases differ or their $120^\circ$ phase angles shift. 

### Mathematically

When a system becomes unbalanced, it is mathmatically broken down into three symmetrical component systems using Fortescue's Theorem. [[^2]](#footnote-2):

[^2]: Any unbalanced set of N polyphase phasors can be broken down into N symmetrical sets of balanced phasors. It is used for three-phase systems to resolve unbalanced voltages and currents into: positive-sequence, negative sequence, and zero-sequence components.

- Positive-sequence components: Three equal vectors shifted by $120^\circ$ rotating in normal order (A→B→C)

- Negative-sequence components: Three equal vectors shifted by $120^\circ$ rotating in reverse order (A→C→B)

- Zero-sequence components: Three identical vectors with the same magnitude and no phase shift($0^\circ$)

### Physical Effects

The phase currects no longer sum to zero. The current no longer sums up to zero and the leftover current flows into a dedicated neutral wire back to the source. Negative-sequence voltages create a counter-rotating magnetic field in motors that act like a brake, producing counter-torque, mechanical vibration, and destructive winding heat. Increased $I^2R$ power losses occur across transmission lines and distribution networks.

### Common Causes

- Unequal load distribution: connecting too many single-phase loads (homes or offices) to one specitic phase.

- Fault conditions: Line-to-ground or line-to-line faults that temporarily drag down the coltage of an individual phase.

- Blown fuses: A blown fuse on a single phase creates an extreme single-phasing condition for three-phase equipment.