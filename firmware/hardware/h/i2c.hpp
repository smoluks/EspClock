#pragma once

#define MODBUS_READ_INPUT 0x04
#define MODBUS_WRITE_COIL 0x05

bool I2CInit();
inline void I2CSetSpeed(uint32_t speed);

bool I2CReadRegister(uint8_t address, uint8_t reg, uint8_t* value);
bool I2CReadRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length);
bool I2CWriteRegister(uint8_t address, uint8_t reg, uint8_t value);
bool I2CWriteRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length);

bool ModbusOverI2CWriteSingleCoil(uint8_t addr, uint16_t reg, bool value);
bool ModbusOverI2CRead(uint8_t addr, uint16_t reg, uint16_t *value);

bool I2CRead(uint8_t addr, uint8_t* data);
bool I2CReadBuffer(uint8_t addr, uint8_t* buffer, size_t quantity);
bool I2CWriteBuffer(uint8_t addr, const uint8_t *data, size_t quantity);