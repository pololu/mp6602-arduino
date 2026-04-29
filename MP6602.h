// Copyright Pololu Corporation.  For more information, see http://www.pololu.com/

/// \file MP6602.h
///
/// This is the main header file for the MP6602 library,
/// a library for controlling the MP6602 stepper motor driver.
///
/// For more information about this library, see:
///
///   https://github.com/pololu/mp6602-arduino
///
/// That is the main repository for this library.

#pragma once

#include <Arduino.h>
#include <SPI.h>

/// Addresses of control and status registers.
/// The datasheet incudes the read/write bit in the address,
/// so the addresses given here are right shifted by 1 to
/// remove that read/write bit.
enum class MP6602RegAddr : uint8_t
{
  CTRL  = 0x00,
  CTRL2 = 0x01,
  ISET  = 0x02,
  STALL = 0x03,
  BEMF  = 0x04,
  TSTP  = 0x05,
  OCP   = 0x06,
  FAULT = 0x07,
};


/// This class provides low-level functions for reading and writing from the SPI
/// interface of a MP6602 stepper motor controller IC.
///
/// Most users should use the MP6602 class, which provides a
/// higher-level interface, instead of this class.
class MP6602SPI
{
public:
  /// Configures this object to use the specified pin as a chip select pin.
  ///
  /// You must use a chip select pin; the MP6602 requires it.
  void setChipSelectPin(uint8_t pin)
  {
    csPin = pin;
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
  }

  /// Reads the register at the given address and returns its raw value.
  uint16_t readReg(uint8_t address)
  {
    // Arduino out / MP6602 in: first 3 bits = address, 4th bit = R/W (0 for
    // read); last 12 bits ignored
    // Arduino in / MP6602 out: first 4 bits undefined, then 12 bits of data read
    // out

    selectChip();
    uint16_t data = transfer16((uint16_t)(address & 0x7) << 13) & 0xFFF;
    deselectChip();
    return data;
  }

  /// Reads the register at the given address and returns its raw value.
  uint16_t readReg(MP6602RegAddr address)
  {
    return readReg((uint8_t)address);
  }

  /// Writes the specified value to a register.
  void writeReg(uint8_t address, uint16_t value)
  {
    // Arduino out / MP6602 in: first 3 bits = address, 4th bit = R/W (1 for
    // write), then 12 data bits to be written
    // Arduino in / MP6602 out: undefined

    selectChip();
    transfer16(((uint16_t)(address & 0x7) << 13) | (1 << 12) | (value & 0xFFF));
    deselectChip();
  }

  /// Writes the specified value to a register.
  void writeReg(MP6602RegAddr address, uint16_t value)
  {
    writeReg((uint8_t)address, value);
  }

private:

  // Max SPI frequency is 10 MHz. We use half that (5 MHz) to leave some margin
  // for timing inaccuracies.
  SPISettings settings = SPISettings(5000000, MSBFIRST, SPI_MODE0);

  uint16_t transfer16(uint16_t value)
  {
    return SPI.transfer16(value);
  }

  void selectChip()
  {
    digitalWrite(csPin, LOW);
    SPI.beginTransaction(settings);
  }

  void deselectChip()
  {
   SPI.endTransaction();
   digitalWrite(csPin, HIGH);
  }

  uint8_t csPin;
};


/// Possible arguments to setStepMode().
enum class MP6602StepMode : uint8_t
{
  MicroStep1     = 0b000, ///< Full step with 71% current
  MicroStep2     = 0b001, ///< 1/2-step
  MicroStep4     = 0b010, ///< 1/4-step
  MicroStep8     = 0b011, ///< 1/8-step
  MicroStep16    = 0b100, ///< 1/16-step
  MicroStep32    = 0b101, ///< 1/32-step
};

/// Bits that are set in the return value of readFault() to indicate warning and
/// fault conditions.
///
/// See the MP6602 datasheet for detailed descriptions of these conditions.
enum class MP6602FaultBit : uint8_t
{
  STALL = 9, ///< Rotor stall
  OLA   = 8, ///< Open load on bridge A
  OLB   = 7, ///< Open load on bridge B
  OCP   = 6, ///< Over-current protection event (logical OR of bits in OCP register; read-only)
  OTS   = 5, ///< Over-temperature shutdown
  OTW   = 4, ///< Over-temperature warning
  OVP   = 3, ///< Over-voltage protection
  VINUV = 2, ///< VIN under-voltage
  VCCUV = 1, ///< VCC under-voltage (set at start-up and reset)
  FLT   = 0, ///< Fault indication (logical OR of other bits in FAULT register except VCCUV and VINUV; 0 when nFAULT pin is high, 1 when low)
};

/// Bits that are set in the return value of readOCP() to indicate over-current
/// protection events.
///
/// See the MP6602 datasheet for detailed descriptions of these conditions.
enum class MP6602OCPBit : uint8_t
{
  OCPAH = 3, ///< Bridge A high-side OCP (such as short to ground)
  OCPAL = 2, ///< Bridge A low-side OCP (such as short to VIN)
  OCPBH = 1, ///< Bridge B high-side OCP (such as as short to ground)
  OCPBL = 0, ///< Bridge B low-side OCP (such as short to VIN)
};


/// This class provides high-level functions for controlling an MP6602 stepper
/// motor driver.
class MP6602
{
public:
  /// The default constructor.
  MP6602()
  {
    // All settings set to power-on defaults
    ctrl  = 0x118;
    ctrl2 = 0x034;
    iset  = 0x249;
    stall = 0x800;
    bemf  = 0x300;
  }

  /// Configures this object to use the specified pin as a chip select pin.
  /// You must use a chip select pin; the MP6602 requires it.
  void setChipSelectPin(uint8_t pin)
  {
    driver.setChipSelectPin(pin);
  }

  /// Changes all of the driver's settings back to their default values.
  ///
  /// It is good to call this near the beginning of your program to ensure that
  /// there are no settings left over from an earlier time that might affect the
  /// operation of the driver.
  void resetSettings()
  {
    ctrl  = 0x118;
    ctrl2 = 0x034;
    iset  = 0x249;
    stall = 0x800;
    bemf  = 0x300;
    applySettings();

    // Also restore the TSTP register (indexer step table position) to its reset
    // value.
    driver.writeReg(MP6602RegAddr::TSTP, 0x510);
  }

  /// Reads back the SPI configuration registers from the device and verifies
  /// that they are equal to the cached copies stored in this class.
  ///
  /// This can be used to verify that the driver is powered on and has not lost
  /// them due to a power failure.  Only registers that contain driver settings
  /// are verified (and bits that do not contain settings are masked off and not
  /// verified).
  ///
  /// @return 1 if the settings from the device match the cached copies, 0 if
  /// they do not.
  bool verifySettings()
  {
    return driver.readReg(MP6602RegAddr::CTRL) == (ctrl & ~(1 << 1)) && // Bit 1 (STEP) is automatically reset to 0 (always reads as 0)
           driver.readReg(MP6602RegAddr::CTRL2) == ctrl2 &&
           driver.readReg(MP6602RegAddr::ISET)  == iset &&
           driver.readReg(MP6602RegAddr::STALL) == stall &&
           (driver.readReg(MP6602RegAddr::BEMF) & 0xF00) == (bemf & 0xF00); // Only verify bits 11:8, as bits 7:0 are BEMF measured value (not a driver setting) and read-only
  }

  /// Re-writes the cached settings stored in this class to the device.
  ///
  /// You should not normally need to call this function because settings are
  /// written to the device whenever they are changed.  However, if
  /// verifySettings() returns false (due to a power interruption, for
  /// instance), then you could use applySettings() to get the device's settings
  /// back into the desired state.
  void applySettings()
  {
    writeCachedReg(MP6602RegAddr::CTRL2);
    writeCachedReg(MP6602RegAddr::ISET);
    writeCachedReg(MP6602RegAddr::STALL);
    writeCachedReg(MP6602RegAddr::BEMF);

    // CTRL is written last because it contains the EN bit, and we want to try
    // to have all the other settings correct first.
    writeCachedReg(MP6602RegAddr::CTRL);
  }


  /// Sets the driver's current limit during stepping, in units of milliamps.
  ///
  /// This function tries to pick the closest current limit that is equal to or
  /// lower than the desired one. The available settings range from 0.125 A to 4
  /// A in increments of 125 mA.
  void setCurrentMilliamps(uint16_t current)
  {
    if (current > 4000) { current = 4000; }
    if (current < 125) { current = 125; }

    uint8_t is = (current - 125) / 125;
    iset = (iset & 0xFC0) | is;
    writeCachedReg(MP6602RegAddr::ISET);
  }

  /// Sets the driver's current limit during automatic hold mode, in units of
  /// milliamps.
  ///
  /// This setting only applies if automatic hold mode is enabled, which you can
  /// do with with setAutoHoldTimeMs(). This function tries to pick the
  /// closest current limit that is equal to or lower than the desired one. The
  /// available settings range from 0.125 A to 4 A in increments of 125 mA.
  void setHoldCurrentMilliamps(uint16_t current)
  {
    if (current > 4000) { current = 4000; }
    if (current < 125) { current = 125; }

    uint8_t il = (current - 125) / 125;
    iset = (iset & 0x03F) | ((uint16_t)il << 6);
    writeCachedReg(MP6602RegAddr::ISET);
  }

  /// Sets the time that should pass after the last step, in milliseconds,
  /// before the driver's automatic hold current feature activates and it
  /// switches to the configured holding current.
  ///
  /// This function takes an integer value and tries to pick the closest time
  /// period that is equal to or higher than the desired one. The available
  /// settings range from 15.6 ms to 1000 ms (1 s). See Table 5 in the MP6602
  /// datasheet for the available settings. Calling this function with an
  /// argument of 0 disables the automatic hold current feature.
  void setAutoHoldTimeMs(uint16_t time)
  {
    uint8_t ah;

    if      (time ==   0) { ah = 0; } // disabled
    else if (time <=  15) { ah = 1; } //   15.6 ms
    else if (time <=  31) { ah = 2; } //   31.3 ms
    else if (time <=  62) { ah = 3; } //   62.5 ms
    else if (time <= 125) { ah = 4; } //  125   ms
    else if (time <= 250) { ah = 5; } //  250   ms
    else if (time <= 500) { ah = 6; } //  500   ms
    else                  { ah = 7; } // 1000   ms

    ctrl = (ctrl & 0x1FF) | ((uint16_t)ah << 9);
    writeCachedReg(MP6602RegAddr::CTRL);
  }

  /// Enables the driver (EN = 1).
  ///
  /// You can use this function to enable the driver and simply leave the ENBL
  /// (EN) pin disconnected. If ENBL is high, calling this function will disable
  /// the driver instead (the driver state is controlled by the logical XOR of
  /// the EN bit and the value of the ENBL pin).
  void enableDriver()
  {
    ctrl |= 1;
    writeCachedReg(MP6602RegAddr::CTRL);
  }

  /// Disables the driver (EN = 0).
  ///
  /// You can use this function to disable the driver and simply leave the ENBL
  /// (EN) pin disconnected. If ENBL is high, calling this function will enable
  /// the driver instead (the driver state is controlled by the logical XOR of
  /// the EN bit and the value of the ENBL pin).
  void disableDriver()
  {
    ctrl &= ~1;
    writeCachedReg(MP6602RegAddr::CTRL);
  }

  /// Sets the motor direction (DIR).
  ///
  /// Allowed values are 0 or 1.
  ///
  /// You can use this command to control the direction of the stepper motor and
  /// leave the DIR pin disconnected. (The direction is controlled by the
  /// logical XOR of the DIR bit and the value of the DIR pin.)
  void setDirection(bool value)
  {
    if (value)
    {
      ctrl |= (1 << 2);
    }
    else
    {
      ctrl &= ~(1 << 2);
    }
    writeCachedReg(MP6602RegAddr::CTRL);
  }

  /// Returns the cached value of the motor direction (DIR).
  ///
  /// This does not perform any SPI communication with the driver.
  bool getDirection()
  {
    return (ctrl >> 2) & 1;
  }

  /// Advances the indexer by one step (STEP = 1).
  ///
  /// The driver automatically clears the STEP bit after it is written.
  void step()
  {
    driver.writeReg(MP6602RegAddr::CTRL, ctrl | (1 << 1));
  }

  /// Sets the driver's step mode (MS).
  ///
  /// This affects many things about the performance of the motor, including how
  /// much the output moves for each step taken and how much current flows
  /// through the coils in each stepping position. Available microstepping modes
  /// range from full step to 1/32-step.
  ///
  /// If an invalid step mode is passed to this function, then it selects
  /// 1/8-step mode, which is the driver's default.
  ///
  /// Example usage:
  /// ~~~{.cpp}
  /// sd.setStepMode(MP6602StepMode::MicroStep32);
  /// ~~~
  void setStepMode(MP6602StepMode mode)
  {
    if (mode > MP6602StepMode::MicroStep32)
    {
      // Invalid mode; pick 1/8-step by default.
      mode = MP6602StepMode::MicroStep8;
    }

    ctrl = (ctrl & 0xFC7) | ((uint8_t)mode << 3);
    writeCachedReg(MP6602RegAddr::CTRL);
  }

  /// Sets the driver's stepping mode (MS).
  ///
  /// This version of the function allows you to express the requested
  /// microstepping ratio as a number directly.
  ///
  /// Example usage:
  /// ~~~{.cpp}
  /// sd.setStepMode(32);
  /// ~~~
  void setStepMode(uint16_t mode)
  {
    MP6602StepMode sm;

    switch (mode)
    {
      case 1:   sm = MP6602StepMode::MicroStep1;   break;
      case 2:   sm = MP6602StepMode::MicroStep2;   break;
      case 4:   sm = MP6602StepMode::MicroStep4;   break;
      case 8:   sm = MP6602StepMode::MicroStep8;   break;
      case 16:  sm = MP6602StepMode::MicroStep16;  break;
      case 32:  sm = MP6602StepMode::MicroStep32;  break;

      // Invalid mode; pick 1/8-step by default.
      default:  sm = MP6602StepMode::MicroStep8;
    }

    setStepMode(sm);
  }

  /// Reads the FAULT status register of the driver.
  ///
  /// The return value is a 10-bit unsigned integer that has one bit for each
  /// FAULT condition.  You can simply compare the return value to 0 to see if
  /// any of the bits are set, or you can use the logical AND operator (`&`) and
  /// the #MP6602FaultBit enum to check individual bits. Fault bits are latched
  /// and remain set until cleared by writing a 1 to the appropriate bit (you
  /// can call clearFaults() to clear all faults).
  ///
  /// Example usage:
  /// ~~~{.cpp}
  /// if (sd.readFault() & ((uint16_t)1 << MP6602FaultBit::OVP))
  /// {
  ///   // Over-voltage protection fault has been triggered.
  /// }
  /// ~~~
  uint16_t readFault()
  {
    return driver.readReg(MP6602RegAddr::FAULT) & 0x3FF;
  }

  /// Reads the OCP status register of the driver.
  ///
  /// The return value is a 4-bit unsigned integer that has one bit for each OCP
  /// condition.  You can simply compare the return value to 0 to see if any of
  /// the bits are set, or you can use the logical AND operator (`&`) and the
  /// #MP6602OCPBit enum to check individual bits. OCP bits are latched
  /// and remain set until cleared by writing a 1 to the appropriate bit (you
  /// can call clearFaults() to clear all faults, including OCP faults).
  uint8_t readOCP()
  {
    return driver.readReg(MP6602RegAddr::OCP) & 0xF;
  }

  /// Clears any fault conditions that are currently latched in the driver,
  /// including bits in the OCP register.
  ///
  /// WARNING: Calling this function clears latched faults, which might allow
  /// the motor driver outputs to reactivate.  If you do this repeatedly without
  /// fixing an abnormal condition (like a short circuit), you might damage the
  /// driver.
  void clearFaults()
  {
    // Write 1 to all OCP flag bits. Upper 8 bits of OCP are reserved, so keep
    // them the same as reset values.
    driver.writeReg(MP6602RegAddr::OCP, 0x55F);

    // Write 1 to all writable fault flag bits. Bits 11:10 are reserved, and OCP
    // (6) and FLT (0) are read-only (OR-ed from other bits).
    driver.writeReg(MP6602RegAddr::FAULT, 0x3BE);
  }

  /// Gets the cached value of a register. If the given register address is not
  /// valid, this function returns 0.
  uint16_t getCachedReg(MP6602RegAddr address)
  {
    uint16_t * cachedReg = cachedRegPtr(address);
    if (!cachedReg) { return 0; }
    return *cachedReg;
  }

  /// Writes the specified value to a register after updating the cached value
  /// to match.
  ///
  /// Using this function keeps this object's cached settings consistent with
  /// the settings being written to the driver, so if you are using
  /// verifySettings(), applySettings(), and/or any of the other functions for
  /// specific settings that this library provides, you should use this function
  /// for direct register accesses instead of calling MP6602SPI::writeReg()
  /// directly.
  void setReg(MP6602RegAddr address, uint16_t value)
  {
    uint16_t * cachedReg = cachedRegPtr(address);
    if (!cachedReg) { return; }
    *cachedReg = value & 0xFFF;
    driver.writeReg(address, value);
  }

protected:

  uint16_t ctrl, ctrl2, iset, stall, bemf;

  /// Returns a pointer to the variable containing the cached value for the
  /// given register.
  uint16_t * cachedRegPtr(MP6602RegAddr address)
  {
    switch (address)
    {
      case MP6602RegAddr::CTRL:  return &ctrl;
      case MP6602RegAddr::CTRL2: return &ctrl2;
      case MP6602RegAddr::ISET:  return &iset;
      case MP6602RegAddr::STALL: return &stall;
      case MP6602RegAddr::BEMF:  return &bemf;
      default: return nullptr;
    }
  }

  /// Writes the cached value of the given register to the device.
  void writeCachedReg(MP6602RegAddr address)
  {
    uint16_t * cachedReg = cachedRegPtr(address);
    if (!cachedReg) { return; }
    driver.writeReg(address, *cachedReg);
  }

public:
  /// This object handles all the communication with the MP6602.  Generally, you
  /// should not need to use it in your code for basic usage of the driver, but
  /// you might want to use it to access more advanced settings that the MP6602
  /// class does not provide functions for.
  MP6602SPI driver;
};