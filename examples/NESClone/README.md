# NES Clone

This example can used to connect the following controllers:

* PolyStation

<img src="1.png" alt="ZX Spectrum Joystick" width=250/></img></th>

## Credits
* Modified version: [Wolf](https://github.com/Wolf359Stella)

## Pinout

<center><img src="3.png" alt="ZX Spectrum Joystick"/></center>

 * The gamepad does not follow the same protocol as the SNES.
 * It has more buttons, but only 10 are useful simulatenously (smaller than SNES's). This is not a hardware limitation (talking about the gamepad). The embedded asic generates a internal 15 Hz clock and make some signal filtering to get the following behaviors.
    * X, &#9723;, L1 and L2 have the same output bit.
    * &#9723; and L1 only sets once every 2 cycles.
    * &#9651;, &#9711;, R1 and R2	has the same output.
    * &#9651; and R1 only sets once every 2 cycles


## Material
<center>
   <h3>Protocol package<h3>
   <img src="4.png" alt="Package" width=500/>
   <h3>Protocol package vizualization <h3>
   <img src="2.png" alt="Waveform" width=500/>
</center>