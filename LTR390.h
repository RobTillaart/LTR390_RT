#pragma once
//
//    FILE: LTR390.h
//  AUTHOR: Rob Tillaart
//    DATE: 2024-04-29
// VERSION: 0.2.0
// PURPOSE: Arduino library for the I2C LTR390 UV sensor.
//     URL: https://github.com/RobTillaart/LTR390_RT

#include "Arduino.h"
#include "Wire.h"

#define LTR390_LIB_VERSION (F("0.2.0"))

//  LTR390 ERROR CODES
//  TODO

//  LTR390 REGISTERS (page 13 datasheet)
namespace LTR390RT_REGISTER
{
  constexpr uint8_t LTR390_MAIN_CTRL             = 0x00;
  constexpr uint8_t LTR390_ALS_UVS_MEAS_RATE     = 0x04;
  constexpr uint8_t LTR390_ALS_UVS_GAIN          = 0x05;
  constexpr uint8_t LTR390_PART_ID               = 0x06;
  constexpr uint8_t LTR390_MAIN_STATUS           = 0x07;

  constexpr uint8_t LTR390_ALS_DATA_0            = 0x0D;
  constexpr uint8_t LTR390_ALS_DATA_1            = 0x0E;
  constexpr uint8_t LTR390_ALS_DATA_2            = 0x0F;

  constexpr uint8_t LTR390_UVS_DATA_0            = 0x10;
  constexpr uint8_t LTR390_UVS_DATA_1            = 0x11;
  constexpr uint8_t LTR390_UVS_DATA_2            = 0x12;

  constexpr uint8_t LTR390_INT_CFG               = 0x19;
  constexpr uint8_t LTR390_INT_PST               = 0x1A;

  constexpr uint8_t LTR390_ALS_UVS_THRES_UP_0    = 0x21;
  constexpr uint8_t LTR390_ALS_UVS_THRES_UP_1    = 0x22;
  constexpr uint8_t LTR390_ALS_UVS_THRES_UP_2    = 0x23;

  constexpr uint8_t LTR390_ALS_UVS_THRES_LOW_0   = 0x24;
  constexpr uint8_t LTR390_ALS_UVS_THRES_LOW_1   = 0x25;
  constexpr uint8_t LTR390_ALS_UVS_THRES_LOW_2   = 0x26;
}

namespace LTR390_MAIN_CTRL
{
  constexpr uint8_t ENABLE   = 0x02; // Bit 1: ALS/UVS Enable
  constexpr uint8_t UVS_MODE = 0x08; // Bit 3: 1 = UV, 0 = ALS
  constexpr uint8_t SW_RESET = 0x10; // Bit 4: Software reset

  constexpr uint8_t ALS_ACTIVE = ENABLE;                 // ALS mode (UVS_MODE = 0)
  constexpr uint8_t UVS_ACTIVE = ENABLE | UVS_MODE;      // UV mode
}

namespace LTR390_I2C
{
  constexpr uint8_t ADDRESS = 0x53;
}

class LTR390
{
public:
  explicit LTR390(TwoWire *wire = &Wire):
    _wire{wire},
    _address{LTR390_I2C::ADDRESS},
    _resolution{2},
    _rate{2},
    _gain{1}
  {}

  bool begin()
  {
    return isConnected();
  }

  bool isConnected() const
  {
    _wire->beginTransmission(_address);
    return (_wire->endTransmission() == 0);
  }

  uint8_t getAddress() const
  {
    return _address;
  }

  //////////////////////////////////////////////
  //
  //  MAIN CONTROL
  //
  void setALSMode()
  {
    writeRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL, LTR390_MAIN_CTRL::ALS_ACTIVE);
  }

  void setUVSMode()
  {
    writeRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL, LTR390_MAIN_CTRL::UVS_ACTIVE);
  }

  uint8_t reset()
  {
    writeRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL, LTR390_MAIN_CTRL::SW_RESET);
    delay(100);
    return readRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL);
  }

  void enable()
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL);
    reg |= LTR390_MAIN_CTRL::ENABLE;
    writeRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL, reg);
  }

  void disable()
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL);
    reg &= ~LTR390_MAIN_CTRL::ENABLE;
    writeRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL, reg);
  }

  bool isEnabled() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_MAIN_CTRL);
    return (reg & LTR390_MAIN_CTRL::ENABLE) > 0;
  }

  //////////////////////////////////////////////
  //
  //  MEASUREMENT CONFIGURATION
  //
  void setResolution(uint8_t resolution) //  resolution = 0..5 (2 default)
  {
    if (resolution > 5)
      resolution = 5;
    _resolution = resolution;
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_MEAS_RATE);
    reg &= 0x07;
    reg |= (resolution << 4);
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_MEAS_RATE, reg);
  }

  uint8_t getResolution() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_MEAS_RATE);
    return (reg >> 4) & 0x07;
  }

  float getIntegrationTime() const
  {
    static constexpr uint16_t intTime[6] = {800, 400, 200, 100, 50, 25};
    return intTime[_resolution] * 0.5f;
  }

  //////////////////////////////////////////////

  void setRate(uint8_t rate) //  rate = 0..7 (2 default)
  {
    if (rate > 7)
      rate = 7;
    _rate = rate;
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_MEAS_RATE);
    reg &= 0xF8;
    reg |= rate;
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_MEAS_RATE, reg);
  }

  uint8_t getRate() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_MEAS_RATE);
    return reg & 0x07;
  }

  float getMeasurementTime() const
  {
    static constexpr uint16_t measTime[8] = {25, 50, 100, 200, 500, 1000, 2000, 2000};
    return measTime[_rate];
  }

  //////////////////////////////////////////////

  void setGain(uint8_t gain) //  gain = 0..4
  {
    if (gain > 4)
      gain = 4;
    _gain = gain;
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_GAIN);
    reg &= 0xF8;
    reg |= gain;
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_GAIN, reg);
  }

  uint8_t getGain() const
  {
    //  return _gain; // from cache
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_GAIN);
    return reg & 0x07;
  }

  uint8_t getGainFactor() const
  {
    static constexpr uint8_t gainFactor[5] = {1, 3, 6, 9, 18};
    return gainFactor[_gain];
  }

  //////////////////////////////////////////////
  //
  //  PART_ID
  //
  uint8_t getPartID() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_PART_ID);
    return reg >> 4;
  }

  uint8_t getRevisionID() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_PART_ID);
    return reg & 0x0F;
  }

  //////////////////////////////////////////////
  //
  //  MAIN STATUS
  //
  uint8_t getStatus() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_MAIN_STATUS);
    return reg & 0x38;
  }

  bool getPowerOnStatus() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_MAIN_STATUS);
    return (reg & 0x20) > 0;
  }

  bool getInterruptStatus() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_MAIN_STATUS);
    return (reg & 0x10) > 0;
  }

  bool getDataStatus() const
  {
    uint8_t reg = readRegister(LTR390RT_REGISTER::LTR390_MAIN_STATUS);
    return (reg & 0x08) > 0;
  }

  //////////////////////////////////////////////
  //
  //  GET DATA
  //
  uint32_t getALSData() const
  {
    uint32_t value = readRegister(LTR390RT_REGISTER::LTR390_ALS_DATA_2) & 0x0F;
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_ALS_DATA_1);
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_ALS_DATA_0);
    return value;
  }

  uint32_t getUVSData() const
  {
    uint32_t value = readRegister(LTR390RT_REGISTER::LTR390_UVS_DATA_2) & 0x0F;
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_UVS_DATA_1);
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_UVS_DATA_0);
    return value;
  }

  float getLUX(float windowsFactor = 1.0f) const
  {
    float lux = (100 * 0.6f) * getALSData();
    lux /= (getGainFactor() * getIntegrationTime());
    if (windowsFactor > 1.0f)
      lux *= windowsFactor;
    return lux;
  }

  float getUVIndex(float windowsFactor = 1.0) const
  {
    float reciprokeSensitivity = (18 * 400) / 2300.0f;
    reciprokeSensitivity /= (getGainFactor() * getIntegrationTime());
    uint32_t uvi = getUVSData() * reciprokeSensitivity;
    if (windowsFactor > 1.0)
      uvi *= windowsFactor;
    return uvi;
  }

  //////////////////////////////////////////////
  //
  //  INTERRUPT
  //  TODO elaborate
  //
  int setInterruptConfig(uint8_t value)
  {
    return writeRegister(LTR390RT_REGISTER::LTR390_INT_CFG, value);
  }

  uint8_t getInterruptConfig() const
  {
    return readRegister(LTR390RT_REGISTER::LTR390_INT_CFG);
  }

  int setInterruptPersist(uint8_t value)
  {
    return writeRegister(LTR390RT_REGISTER::LTR390_INT_PST, value);
  }

  uint8_t getInterruptPersist() const
  {
    return readRegister(LTR390RT_REGISTER::LTR390_INT_PST);
  }

  //////////////////////////////////////////////
  //
  //  THRESHOLD
  //  TODO elaborate
  //
  void setHighThreshold(uint32_t value)
  {
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_UP_0, value & 0xFF);
    value >>= 8;
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_UP_1, value & 0xFF);
    value >>= 8;
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_UP_2, value & 0x0F);
  }

  uint32_t getHighThreshold() const
  {
    uint32_t value = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_UP_2) & 0x0F;
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_UP_1);
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_UP_0);
    return value;
  }

  void setLowThreshold(uint32_t value)
  {
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_LOW_0, value & 0xFF);
    value >>= 8;
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_LOW_1, value & 0xFF);
    value >>= 8;
    writeRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_LOW_2, value & 0x0F);
  }

  uint32_t getLowThreshold() const
  {
    uint32_t value = readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_LOW_2) & 0x0F;
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_LOW_1);
    value <<= 8;
    value += readRegister(LTR390RT_REGISTER::LTR390_ALS_UVS_THRES_LOW_0);
    return value;
  }

  //////////////////////////////////////////////
  //
  //  PRIVATE
  //
  int writeRegister(uint8_t reg, uint8_t value)
  {
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->write(value);
    int n = _wire->endTransmission();
    // if (n != 0)
    //{
    //   Serial.print("write:\t");
    //   Serial.println(n);
    // }
    return n;
  }

  uint8_t readRegister(uint8_t reg) const
  {
    _wire->beginTransmission(_address);
    _wire->write(reg);
    int n = _wire->endTransmission();
    // if (n != 0)
    //{
    //   Serial.print("read:\t");
    //   Serial.println(n);
    //   return n;
    // }

    n = _wire->requestFrom(_address, (uint8_t)1);
    if (n != 1)
    {
      //  Serial.print("requestFrom: \t");
      //  Serial.print(n);
      //  return n;
    }
    return _wire->read();
  }

private:
  TwoWire *_wire;
  uint8_t _address;

  uint8_t _resolution;
  uint8_t _rate;
  uint8_t _gain;
};

//  -- END OF FILE --
