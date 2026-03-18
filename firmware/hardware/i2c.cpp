#include <Wire.h>
#include "h/i2c.hpp"
#include "h/systick.hpp"
#include "../h/hardware.hpp"

static const char *I2C_TAG = "I2C";

static bool tryWrite(uint8_t data);
static bool tryWrite(const uint8_t *data, size_t quantity);
static bool tryEndTransmission();

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

bool I2CReadRegister(uint8_t address, uint8_t reg, uint8_t *value)
{
    // Write address of register
    Wire.beginTransmission(address);
    bool regResult = tryWrite(reg);
    bool endResult = tryEndTransmission();
    if(!regResult || !endResult)
    {
        return false;
    }

    // Read registers
    return I2CRead(address, value);
}

bool I2CReadRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length)
{
    // Write address of register
    Wire.beginTransmission(address);
    bool regResult = tryWrite(startReg);
    bool endResult = tryEndTransmission();
    if(!regResult || !endResult)
    {
        return false;
    }

    // Read registers
   return I2CReadBuffer(address, buffer, length);
}

bool I2CWriteRegister(uint8_t address, uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(address);
    bool regResult = tryWrite(reg);
    bool valueResult = tryWrite(value);
    bool endResult = tryEndTransmission();
    return regResult && valueResult && endResult;
}

bool I2CWriteRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length)
{
    Wire.beginTransmission(address);
    bool regResult = tryWrite(startReg);
    bool bufferResult = tryWrite(buffer, length);
    bool endResult = tryEndTransmission();
    return regResult && bufferResult && endResult;
}

static uint8_t writeSingleCoilCommand[5] = {MODBUS_WRITE_COIL, 0x00, 0x00, 0x00, 0x00}; // command to read a single register
bool ModbusOverI2CWriteSingleCoil(uint8_t addr, uint16_t reg, bool value)
{
    writeSingleCoilCommand[1] = byte(reg >> 8);
    writeSingleCoilCommand[2] = byte(reg & 0xFF);  
    writeSingleCoilCommand[3] = value ? 0xFF : 0x00; // set coil value

    return I2CWriteBuffer(addr, writeSingleCoilCommand, sizeof(writeSingleCoilCommand));
}

static uint8_t readCommand[5] = {MODBUS_READ_INPUT, 0x00, 0x00, 0x00, 0x01}; // command to read a single register
bool ModbusOverI2CRead(uint8_t addr, uint16_t reg, uint16_t *value)
{
    readCommand[1] = byte(reg >> 8);
    readCommand[2] = byte(reg & 0xFF); 
    if (!I2CWriteBuffer(addr, readCommand, sizeof(readCommand)))
    {
        return false;
    }

    delay(5);

    uint8_t data[4];
    if (!I2CReadBuffer(addr, data, sizeof(data)))
    {
        return false;
    }

    if (data[0] != MODBUS_READ_INPUT)
    {
        ESP_LOGW(I2C_TAG, "Modbus read failed, received command 0x%X, reg 0x%X", data[0], reg);
        return false;
    }
    if (data[1] != 2) // length
    {
        ESP_LOGW(I2C_TAG, "Modbus read failed, received length 0x%d", data[1]);
        return false;
    }

    *value = ((data[2] << 8) | data[3]);

    return true;
}

bool I2CReadBuffer(uint8_t addr, uint8_t* buffer, size_t quantity)
{
    Wire.flush();

    size_t result =  Wire.requestFrom(addr, quantity, true);
    if (result != quantity)
    {
        ESP_LOGE(I2C_TAG, "I2C read buffer error, expected count %d, real %d", quantity, result);
        return false;
    }

    char text[64] = "Readed data: 0x";
    for(int i = 0; i < quantity; i++)
    {
        buffer[i] = Wire.read();
        snprintf(text, sizeof(text), "%s%X ", text, buffer[i]);
    }

    ESP_LOGV(I2C_TAG, "I2C received: %s", text);
    return true;
}

bool I2CRead(uint8_t addr, uint8_t* data)
{
    size_t result =  Wire.requestFrom(addr, 1, true);
    if (result != 1)
    {
        ESP_LOGE(I2C_TAG, "I2C read byte error, result %d", result);
        return false;
    }

    *data = Wire.read();
    return true;
}

bool I2CWriteBuffer(uint8_t addr, const uint8_t *data, size_t quantity)
{
    Wire.beginTransmission(addr);
    bool writeResulrt = tryWrite(data, quantity);
    bool endResult = tryEndTransmission();
    return writeResulrt && endResult;
}

static bool tryWrite(uint8_t data)
{
    size_t result = Wire.write(data);
    if (result != 1)
    {
        ESP_LOGE(I2C_TAG, "I2C write byte error, result %d", result);
        return false;
    }

    return true;
}

static bool tryWrite(const uint8_t *data, size_t quantity)
{
    size_t result = Wire.write(data, quantity);
    if (result != quantity)
    {
        ESP_LOGE(I2C_TAG, "I2C write buffer error, expected count %d, real %d", quantity, result);
        return false;
    }

    return true;
}

static bool tryEndTransmission()
{
    uint8_t end = Wire.endTransmission();
    switch (end)
    {
    case 0:
        return true;
    case 1:
        ESP_LOGE(I2C_TAG, "I2C end error: data too long to fit in transmit buffer");
        break;
    case 2:
        ESP_LOGE(I2C_TAG, "I2C end error: received NACK on transmit of address");
        break;
    case 3:
        ESP_LOGE(I2C_TAG, "I2C end error: received NACK on transmit of data");
        break;
    case 4:
        ESP_LOGE(I2C_TAG, "I2C end error: other error");
        break;
    case 5:
        ESP_LOGE(I2C_TAG, "I2C end error: timeout");
        break;
    default:
        ESP_LOGE(I2C_TAG, "I2C end error: unknown error %d", end);
        break;
    }

    return false;
}