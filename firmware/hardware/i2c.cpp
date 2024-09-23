#include <Wire.h>
#include "h/i2c.hpp"
#include "../h/hardware.hpp"
#include "../controllers/h/systick.hpp"

static const char *I2C_TAG = "I2C";

bool I2CInit()
{
    if (!Wire.begin(SDA_PIN, SCL_PIN, 100000))
    {
        ESP_LOGE(I2C_TAG, "I2C init error");
        return false;
    }

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
        ESP_LOGW(I2C_TAG, "I2C read count error, expected %d, real %d, device %X", length, size, address);
        return size;
    }

    uint8_t i = 0;
    while (i < length)
    {
        buffer[i++] = Wire.read();
    }

    return size;
}

size_t I2CReadRegister(uint8_t address, uint8_t reg, uint8_t *value)
{
    // Write address of register
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();

    // Read registers
    uint8_t size = Wire.requestFrom(address, (uint8_t)1);
    if (size != 1)
    {
        ESP_LOGW(I2C_TAG, "I2C read register error, expected %d, real %d, device %X", 1, size, address);
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
        ESP_LOGW(I2C_TAG, "I2C read count error, expected %d, real %d, device %X", length, size, address);
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

int32_t ModbusOverI2CWriteSingleCoil(uint8_t addr, uint16_t reg, bool value)
{
    Wire.beginTransmission(addr);

    bool result =
        Wire.write(MODBUS_WRITE_COIL) &&
        Wire.write(byte(reg >> 8)) &&
        Wire.write(byte(reg & 0xFF)) &&
        Wire.write(value ? 0xFF : 0x00) &&
        !Wire.write(0x00);

    Wire.endTransmission();

    if (!result)
    {
        return -2;
    }

    return 0;
}

int32_t ModbusOverI2CRead(uint8_t addr, uint16_t reg)
{
    Wire.flush();
    Wire.setTimeOut(1000);
    delay(10);

    Wire.beginTransmission(addr);
    bool result = (Wire.write(MODBUS_READ_INPUT) &&
                   Wire.write(byte(reg >> 8)) &&
                   Wire.write(byte(reg & 0xFF)) &&
                   Wire.write(0x00) &&
                   Wire.write(0x01));
    Wire.endTransmission();

    if (!result)
    {
        return -2;
    }

    delay(10);

    if (Wire.requestFrom(addr, 2, false) < 2)
    {
        ESP_LOGW(I2C_TAG, "Modbus read failed, address 0x%X", addr);
        return -1;
    }
    uint8_t command = Wire.read();
    uint8_t size = Wire.read();

    if (command != MODBUS_READ_INPUT)
    {
        ESP_LOGW(I2C_TAG, "Modbus read failed, received command 0x%X, payload:", command);

        Wire.requestFrom(addr, size, true);
        for (uint8_t i = 0; i < size; i++)
        {
            ESP_LOGW(I2C_TAG, "0x%X", (byte)Wire.read());
        }

        return -3;
    }
    if (size != 2) // length
    {
        ESP_LOGW(I2C_TAG, "Modbus read failed, received length 0x%d, payload:", size);

        Wire.requestFrom(addr, size, true);
        for (uint8_t i = 0; i < size; i++)
        {
            ESP_LOGW(I2C_TAG, "0x%X", (byte)Wire.read());
        }

        return -4;
    }

    Wire.requestFrom(addr, 2, true);
    uint8_t high_byte = Wire.read();
    uint8_t low_byte = Wire.read();

    return ((high_byte << 8) | low_byte);
}
