// Changed: 23.01.2022 (������ ������������������)

// Based on code from:
// https://dolman-wim.nl/xmega/book/index.php
// Code 21.8 van pagina 368
// Het bestand i2c.c met de IIC -functies voor het versturen en ontvangen

#include "twi.h"

void i2c_init(TWI_t *twi, uint8_t baudRateRegisterSetting)
{
	twi->MASTER.BAUD   = baudRateRegisterSetting;
	twi->MASTER.CTRLC  = 0;
	twi->MASTER.CTRLA  = TWI_MASTER_ENABLE_bm;
	twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

uint8_t i2c_start(TWI_t *twi, uint8_t address, uint8_t rw)
{
	if ( (twi->MASTER.STATUS & TWI_MASTER_BUSSTATE_gm) !=                      // if bus available
	TWI_MASTER_BUSSTATE_IDLE_gc ) return I2C_STATUS_BUSY; //
	twi->MASTER.ADDR = (address << 1) | rw;                                    // send slave address
	while( ! (twi->MASTER.STATUS & (TWI_MASTER_WIF_bm << rw)) );               // wait until sent

	if ( twi->MASTER.STATUS & TWI_MASTER_RXACK_bm ) {                          // if no ack
		twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		return I2C_STATUS_NO_ACK;
	}

	return I2C_STATUS_OK;
}

uint8_t i2c_restart(TWI_t *twi, uint8_t address, uint8_t rw)
{
	twi->MASTER.ADDR = (address << 1) | rw;                                    // send slave address
	while( ! (twi->MASTER.STATUS & (TWI_MASTER_WIF_bm << rw)) );               // wait until sent

	if ( twi->MASTER.STATUS & TWI_MASTER_RXACK_bm ) {                          // if no ack
		twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		return I2C_STATUS_NO_ACK;
	}

	return I2C_STATUS_OK;
}

void i2c_stop(TWI_t *twi)
{
	twi->MASTER.CTRLC  = TWI_MASTER_CMD_STOP_gc;
	twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

uint8_t i2c_write(TWI_t *twi, uint8_t data)
{
	twi->MASTER.DATA = data;                                                   // send data
	while( ! (twi->MASTER.STATUS & TWI_MASTER_WIF_bm) );                       // wait until sent

	if ( twi->MASTER.STATUS & TWI_MASTER_RXACK_bm ) {                          // if no ack
		twi->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		return I2C_STATUS_NO_ACK;
	}

	return I2C_STATUS_OK;
}

uint8_t i2c_read(TWI_t *twi, uint8_t ack)
{
	uint8_t data;

	while( ! (twi->MASTER.STATUS & TWI_MASTER_RIF_bm) );                  // wait until received
	data = twi->MASTER.DATA;                                              // read data
	twi->MASTER.CTRLC = ((ack==I2C_ACK) ? TWI_MASTER_CMD_RECVTRANS_gc :   // send ack (go on) or
	TWI_MASTER_ACKACT_bm|TWI_MASTER_CMD_STOP_gc);                         // nack (and stop)

	if ( ack == I2C_NACK ) {
		while( ! (twi->MASTER.STATUS & TWI_MASTER_BUSSTATE_IDLE_gc) );
	}

	return data;
}

// my add:
void i2c_read_nbytes(unsigned char *my_buf, const uint8_t n)
{
	uint8_t i;
	for (i=0; i<(n - 1); i++)
	{
		my_buf[i] = i2c_read(&TWIC, I2C_ACK);
	}
	my_buf[n-1] = i2c_read(&TWIC, I2C_NACK);
}
