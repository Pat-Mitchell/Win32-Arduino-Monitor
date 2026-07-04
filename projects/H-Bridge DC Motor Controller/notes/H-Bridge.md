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