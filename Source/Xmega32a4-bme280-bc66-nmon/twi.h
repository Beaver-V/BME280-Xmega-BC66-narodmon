// Changed: 23.01.2022 (удалил закомментированное)

// Based on code from:
// https://dolman-wim.nl/xmega/book/index.php
// Code 21.9 van pagina 369
// Het headerbestand i2c.h met definities en prototypen

#ifndef TWI_H_
#define TWI_H_
#define F_TWI		100000

#include "board3.h"

#define TWI_BAUD(F_SYS, F_TWI)   ((F_SYS / (2 * F_TWI)) - 5)

#define I2C_ACK     0
#define I2C_NACK    1
#define I2C_READ    1
#define I2C_WRITE   0

#define I2C_STATUS_OK      0
#define I2C_STATUS_BUSY    1
#define I2C_STATUS_NO_ACK  2

void    i2c_init(TWI_t *twi, uint8_t baudRateRegisterSetting);
uint8_t i2c_start(TWI_t *twi, uint8_t address, uint8_t rw);
uint8_t i2c_restart(TWI_t *twi, uint8_t address, uint8_t rw);
void    i2c_stop(TWI_t *twi);
uint8_t i2c_write(TWI_t *twi, uint8_t data);
uint8_t i2c_read(TWI_t *twi, uint8_t ack);

// my add:
void i2c_read_nbytes(unsigned char *my_buf, const uint8_t n);

#endif /* TWI_H_ */
