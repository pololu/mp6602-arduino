// This example demonstrates direct access to a control register in the MP6602
// stepper motor driver.
//
// The MP6602 library does not provide dedicated functions for some more
// advanced settings, such as the OT (off time) setting in the CTRL register.
// These settings can still be changed by using the library's setReg() function
// to write a value to the register directly. The setReg() function also updates
// the library object's cached settings to match the value being written to the
// device. This is important to maintain consistency when other library
// functions are later used, and it allows the driver's settings to be verified
// against the cached settings (and the cached settings to be re-applied if
// necessary).
//
// See this library's documentation for information about how to connect the
// driver: https://pololu.github.io/mp6602-arduino

#include <SPI.h>
#include <MP6602.h>

const uint8_t CSPin = 4;

MP6602 sd;

void setup()
{
  Serial.begin(115200);

  SPI.begin();
  sd.setChipSelectPin(CSPin);

  // Give the driver some time to power up.
  delay(1);

  // Reset the driver to its default settings and clear latched fault
  // conditions.
  sd.resetSettings();
  sd.clearFaults();
}

void loop()
{
  uint16_t ctrl = sd.getCachedReg(MP6602RegAddr::CTRL);

  ctrl = (ctrl & 0b111000111111) | ((uint16_t)0b110 << 6); // OT = 0b110: 50 us PWM off time
  sd.setReg(MP6602RegAddr::CTRL, ctrl);
  Serial.println("OT set to 50 us");

  tryVerifySettings();

  delay(1000);

  ctrl = (ctrl & 0b111000111111) | ((uint16_t)0b100 << 6); // OT = 0b100: 40 us PWM off time
  sd.setReg(MP6602RegAddr::CTRL, ctrl);
  Serial.println("OT set to 40 us");

  tryVerifySettings();

  delay(1000);
}

void tryVerifySettings()
{
  if (sd.verifySettings())
  {
    Serial.println("Settings verified");
  }
  else
  {
    Serial.println("Could not verify settings");
  }
}