# MP6602 library for Arduino

[www.pololu.com](https://www.pololu.com/)

## Summary

This is a library for the Arduino IDE that helps interface with an [MP6602
stepper motor driver][mp6602].  It uses the [Arduino SPI][spi] library to
communicate with the SPI interface (nSCS, SCLK, SDATI, and SDATO) of the MP6602.

## Supported platforms

This library is designed to work with the Arduino IDE versions 1.8.x or later;
we have not tested it with earlier versions.  This library should support any
Arduino-compatible board, including the [Pololu A-Star controllers][a-star].

## Getting started

### Hardware

An [MP6602 carrier][mp6602] can be purchased from Pololu's website.  Before
continuing, careful reading of its product page is recommended.

You will need to connect your motor, motor power, and IOREF as described on the
product page.  You should also make the following connections between the
Arduino and the driver:

| Arduino       | MP6602      |
|---------------|-------------|
| Digital pin 2 | DIR         |
| Digital pin 3 | STEP        |
| Digital pin 4 | nSCS        |
| SCK           | SCLK        |
| MOSI          | SDATI (SDI) |
| MISO          | SDATO (SDO) |
| IOREF         | VCC         |
| GND           | GND         |

The SDATO pin is only needed if you want to read information back from the
stepper driver.  Since the motor can be stepped and its direction changed using
the SPI interface, it is possible to use the driver without connecting the STEP
and DIR pins, and they are not used in every example.

The SPI pins (MOSI, MISO, and SCK) on Arduino-compatible boards are sometimes
not labeled.  You should refer to the documentation for your particular board
to find the locations of these pins.

If your Arduino does not have an IOREF pin, connect a supply matching the logic
voltage of your controller (e.g. 5V for an Arduino Uno) to VCC and nSLEEP.

### Software

You can use the Library Manager to install this library:

1. In the Arduino IDE, open the "Sketch" menu, select "Include Library", then
   "Manage Libraries...".
2. Search for "MP6602".
3. Click the MP6602 entry in the list.
4. Click "Install".

If this does not work, you can manually install the library:

1. Download the [latest release archive from GitHub][github] and decompress it.
2. Rename the folder "mp6602-arduino-xxxx" to "MP6602".
3. Drag the "MP6602" folder into the "libraries" directory inside your Arduino
   sketchbook directory.  You can view your sketchbook location by opening the
   "File" menu and selecting "Preferences" in the Arduino IDE.  If there is not
   already a "libraries" folder in that location, you should make the folder
   yourself.
4. After installing the library, restart the Arduino IDE.

## Examples

Several example sketches are available that show how to use the library. You
can access them from the Arduino IDE by opening the "File" menu, selecting
"Examples", and then selecting "MP6602". If you cannot find the examples, the
library was probably installed incorrectly and you should retry the installation
instructions above.

## Documentation

For complete documentation of this library, including many features that were
not mentioned here, see [the mp6602-arduino documentation][doc].  If you are
already on that page, see the MP6602 class reference.

## Version history

* 1.0.0 (2026-04-29): Original release.

[a-star]: https://www.pololu.com/a-star
[doc]: https://pololu.github.io/mp6602-arduino/
[github]: https://github.com/pololu/mp6602-arduino/releases
[mp6602]: https://www.pololu.com/category/378
[spi]: http://www.arduino.cc/en/Reference/SPI
