# Wheatstone Bridge & Strain Gauge

When a force is applied to the free end of a cantilever beam[[^1]](#footnote-1), it causes the beam to bend, creating a strain gradient. The top surface experiences tension and the bottom experiences compression. In this project, a foil strain gauge is mounted on the top surface near the fixed base where strain is at its maximum. The deformation of the strain gauge will cause a minor change in its resistance that will unbalance the wheatstone bridge. Normally too small to yield quantifiable data to an Arduino, an LM358 op-amp is used to amplify the microvolt readings into high resolution readings in the voltage range.

[^1]: A cantilever beam is a horizontal structural element that is firmly fixed or anchored at one end only while the other end remains completely free and unsupported in open space.

## Stress and Strain

At the most fundamental level, mechanics of materials comes down to cause and effect. Stress is the cause (the internale forces acting within a material) and strain is the effect (how the material physically deforms in response).

![Types of Mechanical Stress](Mechanical-Stress.jpg)

### Stress ($\sigma$)

Stress measures the intensity of the internal force inside an object when an external force (load) is applied to it. It is defined as the applied force ($F$) divided by the cross-sectional area ($A$) of the object:

$$\sigma = {F \over A}$$

Resulting in units of $\text {N} / \text {m} ^2$ or Pascals (Pa). Typically, expressed in units of Megapascals (MPa) or Gigapascals (GPa) because a Pascal is a tiny amount of pressure and the focus is when material deforms or fractures.

**Note:** Stress cannot be measured. It's a mathematical concept. It is impossible to place a sensor onto an internal imaginary plane. The observable effect of stress is strain.

### Strain ($\epsilon$)

Strain is the measure of how much an object has deformed relative to its original size. Stretching a 10-inch rubber band by 1 inch stains is a specific amount while stretching a 100-inch bunge cord by 1 inch strains it by a much lower amount.

Strain is the change in length ($\Delta L$) divided by the original length ($L$):

$$\epsilon = {\Delta L \over L}$$

Strain is a dimensionless number. It's often express as a percentage or in "microstrain" (parts per million) for rigid materials like metal or concrete.

### Hooke's Law & Young's Modulus

For most structural materials, applying stress yields a proportional amount of strain. Pulling twice as hard stretches twice as much. This linear relationship is called Hooke's Law:

$$\sigma = E \cdot \epsilon$$

The constant $E$ is Young's Modulus (or the Modulus of Elasticity). It represents the intrinsic "stiffness" of a specific material. Steel has a very high Young's Modulus while rubber has a very low one.

### The Stress-Strain Curve

![The Stress-Strain Curve](Stress-Strain-Curve.jpg)

When a sample is put into an instron machine and pulled slowly until it snaps, the  data is used to plot the stress against the strain resulting in a curve that reveals the material's entire lifecycle under a (mostly static) load.[[^2]](#footnote-2)

[^2]: The diagram used for this section is most likely a mild or low-carbon steel. The small dip in the curve present at the yield point is a phenomenon not present on all other metals. Most metals transition smoothly from elastic to plastic deformation. 

1. Elastic Region (the linear region at the start): The material behaves like a spring. Removing the load in this region results in the material snapping back to its original shape. The slope of this line is Young's Modulus.

2. Yield Point: The critical threshold where the material stops bouncing back. Permanent damage is done to the internal crystal structure.

3. Plastic Region: The material is permanently deforming. Removing the load results in a partial rebound to its original shape. The unloading path is parallel to the linear elastic region on the stress-strain curve and ends at the new deformed position.

4. Ultimate Tensile Strength (The peak): The absolute maximum stress the material can handle before is begins to rapidly fail.

5. Fracture: The material snaps.