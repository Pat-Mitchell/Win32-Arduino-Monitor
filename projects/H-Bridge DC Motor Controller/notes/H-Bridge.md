# H-Bridge

An H-bridge is an electronic circuit that allows for the reversing of current through a load. Because changing the direction of current also changed the direction of a motor's rotation, h-bridges are commonly used as motor drivers in robotics, RC cars, and automated machinery. By changing which switches are open and closed, a DC motor can rotate forward, in reverse, allowed to coast to a stop, or braked. A PWM signal can be used as a voltage supply to control the average voltage across the load.

## H-Bridge Topology

The name comes from the schematic's layout that arranges for switches in the shape of the letter H. The motor (or whatever the load is) sits on the horizontal crossbar between the switches.

       +V (Power Supply)
         |          |
        [S1]      [S3]   <- High-Side Switches
         |    M     |
         +---( )----+      <- Central Bridge (Motor)
         |          |
        [S2]      [S4]   <- Low-Side Switches
         |          |
       Ground     Ground

### Anatomy of the Architecture

- Two vertical legs: The circuit has a left leg and a right leg. Each leg acts as a voltage divider between the positive power supply (+V) and the system ground.

- Four switching elements: Each leg contains two switches connected in series: a **high-side** (S1, S3) that delivers the positive power supply and a **low-side** (S2, S4) connected between the load and the system ground.

### "Shoot-through" Danger

An aspect of h-bridge topology is avoiding a failure state known as shoot-through. If both the top switch and bottom switch on the same leg are turned on at the same time, it creates a direct short cirduit from the power supply to the ground. This instantly destroys the transitors. To prevent this, gate drivers enforce a brief delay called **dead time** where switches are allowed to open and one trasitor completely turns off before its partner turns on.

## DC Motors as Electrical Loads

As electrical loads, DC motors are dynamic and non-linear as compared to LEDs and resistors. They have three primary characteristics:

- Back-EMF: As the motor spins, the rotating armature generates a coltage that opposes the driving voltage called back-EMF (electormotive force). The back-EMF acts as a dynamic resistor, significantly reducing the actual running current drawn during normal operation. Using Ohm's law, the electrical behavior is modeled by:

  $$V_{\text{supply}} - V_{\text{emf}} = I \times R$$

- High starting current: When a motor is stationary, its back-EMF is zero. The result is that the startup current can be much higher than the running current. As the motor RPM increases, the back-emf increases and the net voltage in the circuit decreases. When the motor encounters a mechanical load that brings the RPM to 0 (stall), the back-EMF drops to 0V. It's worth noting that components will heat up with the increased current and be destroyed overtime.

- Inductive kickback: A motor's copper windings act as inductors, meaning they stor magnetic energy. Whenever the h-bridge switches turn off or reverse polarity, the magnetic field collapses, causing a sudden, massive voltage spoke (inductive kickback). These spikes can easily exceed the voltage rating of electrical components (the bridge's transitors in this case) and destroy them.

## Back-EMF and Flyback Diodes

The motor creates an electric field while running. When a switch opens, the magnetic field collapses, triggering a high voltage spike described by $V = L \times {dI \over dt}$.

### The Inductive Voltage Spike

Motor windings act as large inductors. Inductors naturally resist sudden changes in electrical current by storing energy in a magnetic field. When the h-bridge switches open to turn off or reverse the motor, the circuit physically breaks. The current drops from its full running level down to zero almost instantly. This makes the $dI \over dt$ component of the inductive voltage equation a very large negative value. Physically, the inductor generates a very large negative voltage spike (potentially hundreds of volts).

### Flyback Diodes Clamp the Spike

A flyback diode (or freewheeling diode) is placed in parallel across each switching transitor in reverse biased. During normal operation, the diodes block power and do nothing. When switches open, the collapsing magnetic field reverses the motor's voltage polarity. The previously reverse biased diode is now forward biased and provides a loop for the magnetic field to harmlessly diminish over time through heat dissipation.

Without flyback protection, the high voltage spike searches for the path of least resistance to ground and can cause:

- Dielectric breakdown in transistor's silicon junctions.

- Arcing & destruction: Energy literally punches through the internal insulation of the MOSFET or BJT.

- Permanent failure: The stransistor shorts out permanently, killing the h-bridge and potentially back-feeding high voltage into the microcontroller.

## L298N Wiring and Flyback Protection

The L298N bridge purchased for this project does have flyback diodes already soldered in place, removing the need for additional wiring. Eight diodes (four per motor) are visible next to the aluminum heat sink.