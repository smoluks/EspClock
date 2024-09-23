#pragma once

#define MODBUS_READ_INPUT 0x04
#define MODBUS_WRITE_COIL 0x05

bool I2CInit();
inline void I2CSetSpeed(uint32_t speed);
size_t I2CReadBulk(uint8_t address, uint8_t* buffer, uint8_t length);
size_t I2CReadRegister(uint8_t address, uint8_t reg, uint8_t* value);
size_t I2CReadRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length);
size_t I2CWriteRegister(uint8_t address, uint8_t reg, uint8_t value);
size_t I2CWriteBulk(uint8_t address, uint8_t* buffer, uint8_t length);
size_t I2CWriteRegisters(uint8_t address, uint8_t startReg, uint8_t *buffer, uint8_t length);

int32_t ModbusOverI2CWriteSingleCoil(uint8_t addr, uint16_t reg, bool value);
int32_t ModbusOverI2CRead(uint8_t addr, uint16_t reg);