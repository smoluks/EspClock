#include <Wire.h>
#include "h/i2c.hpp"
#include "../h/hardware.hpp"

bool I2CInit()
{
    if (!Wire.begin(SDA_PIN, SCL_PIN, 400000))
    {
        ESP_LOGE("I2C", "I2C init error");
        return false;
    }

    Wire.setTimeOut(100);

    return true;
}

inline void I2CSetSpeed(uint32_t speed)
{
    Wire.setClock(speed);
}

size_t I2CReadBulk(uint8_t address, uint8_t *buffer, uint8_t length)
{
    // Read registers
    uint8_t size = Wire.requestFrom(address, length);
    if (size != length)
    {
        ESP_LOGW("I2C", "I2C read count error, expected %d, real %d, device %X", length, size, address);
        return size;
    }

    uint8_t i = 0;
    while (i < length)
    {
        buffer[i++] = Wire.read();
    }

    return size;
}

size_t I2CReadRegister(uint8_t address, uint8_t reg, uint8_t* value)
{
    // Write address of register
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();

    // Read registers
    uint8_t size = Wire.requestFrom(address, (uint8_t)1);
    if (size != 1)
    {
        ESP_LOGW("I2C", "I2C read register error, expected %d, real %d, device %X", 1, size, address);
        return size;
    }

    *value = Wire.read();

    return size;
}

size_t I2CReadRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length)
{
    // Write address of register
    Wire.beginTransmission(address);
    Wire.write(startReg);
    Wire.endTransmission();

    // Read registers
    uint8_t size = Wire.requestFrom(address, length);
    if (size != length)
    {
        ESP_LOGW("I2C", "I2C read count error, expected %d, real %d, device %X", length, size, address);
        return size;
    }

    uint8_t i = 0;
    while (i < length)
    {
        buffer[i++] = Wire.read();
    }

    return size;
}

size_t I2CWriteBulk(uint8_t address, uint8_t *buffer, uint8_t length)
{
    Wire.beginTransmission(address);
    size_t result = Wire.write(buffer, length);
    Wire.endTransmission();

    return result;
}

size_t I2CWriteRegister(uint8_t address, uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    size_t result = Wire.write(value);
    Wire.endTransmission();

    return result;
}

size_t I2CWriteRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length)
{
    // Write address of register
    Wire.beginTransmission(address);
    Wire.write(startReg);
    size_t result = Wire.write(buffer, length);
    Wire.endTransmission();

    return result;
}

bool ModbusOverI2CWriteSingleCoil(uint8_t addr, uint16_t reg, bool value)
{
    Wire.beginTransmission(addr);

    if(
        !Wire.write(MODBUS_WRITE_COIL) &&
        !Wire.write(reg >> 8) &&
        !Wire.write(reg & 0xFF) &&
        !Wire.write(value ? 0xFF : 0x00) &&
        !Wire.write(0x00))
    return false;

    Wire.endTransmission();

    return true;
}

int32_t ModbusOverI2CRead(uint8_t addr, uint16_t reg)
{
    Wire.beginTransmission(addr);
    if(
        !Wire.write(MODBUS_READ_INPUT) &&
        !Wire.write(addr >> 8) &&
        !Wire.write(addr & 0xFF) &&
        !Wire.write(0x00) &&
        !Wire.write(0x01))
    return -2;
    
    Wire.endTransmission();

    delay(10);

    if (Wire.requestFrom(addr, (uint8_t)4) != 4)
    {
        ESP_LOGW("T6703", "T6703 read failed, address 0x%X", addr);
        return -1;
    }

    uint8_t result[4];
    result[0] = Wire.read();
    result[1] = Wire.read();
    result[2] = Wire.read();
    result[3] = Wire.read();
    return ((result[2] << 8) | result[3]);
}
