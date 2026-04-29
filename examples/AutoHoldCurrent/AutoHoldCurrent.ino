// This example demonstrates the automatic hold current feature of an MP6602
// stepper motor driver, which can be used to automatically lower the motor
// current when the driver is not stepping the motor.
//
// By default, this example sets the stepping and holding currents to 1 A and
// 0.5 A, respectively (although you should change these values as appropriate
// for your system) and configures the automatic hold time to 125 ms. Then, it
// continuously alternates between stepping for 1 second and holding for 1
// second. While the motor is holding, you should see the motor current drop to
// 0.5 A (or your configured holding current) after 1/8 of a second; then, it
// should go back up to 1 A (or your configured stepping current) when the motor
// resumes stepping.
//
// Since SPI is used to trigger steps and set the direction, connecting the
// driver's STEP and DIR pins is optional for this example. However, note that
// using SPI control adds some overhead compared to using the STEP and DIR pins.
//
// Before using this example, be sure to change the setCurrentMilliamps and
// SetHoldCurrentMilliamps lines to have appropriate current limits for your
// system.  Also, see this library's documentation for information about how to
// connect the driver: https://pololu.github.io/mp6602-arduino

#include <SPI.h>
#include <MP6602.h>

const uint8_t CSPin = 4;

// This period is the length of the delay between steps, which controls the
// stepper motor's speed.  You can increase the delay to make the stepper motor
// go slower.  If you decrease the delay, the stepper motor will go faster, but
// there is a limit to how fast it can go before it starts missing steps.
const uint16_t StepPeriodUs = 2000;

MP6602 sd;

void setup()
{
  SPI.begin();
  sd.setChipSelectPin(CSPin);

  // Give the driver some time to power up.
  delay(1);

  // Reset the driver to its default settings and clear latched fault
  // conditions.
  sd.resetSettings();
  sd.clearFaults();

  // Set the current limit while stepping. You should change the number here to an
  // appropriate value for your particular system.
  sd.setCurrentMilliamps(1000);

  // Set the current limit while holding. You should change the number here to an
  // appropriate value for your particular system.
  sd.setHoldCurrentMilliamps(500);

  // Set the time before automatic hold current activates.
  sd.setAutoHoldTimeMs(125);

  // Set the number of microsteps that correspond to one full step.
  sd.setStepMode(MP6602StepMode::MicroStep32);

  // Enable the motor outputs.
  sd.enableDriver();
}

void loop()
{
  // Step in the default direction 1000 times.
  sd.setDirection(0);
  for(unsigned int x = 0; x < 1000; x++)
  {
    sd.step();
    delayMicroseconds(StepPeriodUs);
  }

  // Wait for 1000 ms.
  delay(1000);

  // Step in the other direction 1000 times.
  sd.setDirection(1);
  for(unsigned int x = 0; x < 1000; x++)
  {
    sd.step();
    delayMicroseconds(StepPeriodUs);
  }

  // Wait for 1000 ms.
  delay(1000);
}
