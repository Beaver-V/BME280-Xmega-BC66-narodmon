// Changed: 23.01.2022 (удалил закомментированное)

#ifndef BOARD3_H_
#define BOARD3_H_

#define F_CPU 12000000UL       /* freq 12.0 MHz */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/portpins.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include <stdint.h>

#include "syst_clock.h"
#include "bc66.h"
#include "twi.h"
#include "bme280.h"

// ATXMEGA32A4U pins

// PORTA pins:
// PA0 ... PA4 - not used
#define LED1      PIN5_bm      /* output (green led) */
#define LED2      PIN6_bm      /* output (red LED) */
// #define GSM_PWR   PIN7_bm   /* output for drive power gate */

// PORTB pins:
#define GSM_ON    PIN0_bm      /* output for drive MOSFET gate */
#define GSM_STAT  PIN1_bm      /* input, VDD_EXT from BC66 */
// PB2, PB3 - not used

// PORTC pins:
#define I2C_SDA   PIN0_bm      /* I2C SDA */
#define I2C_SCL   PIN1_bm      /* I2C SCL */
// PC2 - PC7 - not used

// PORTD pins:
// PD0, PD1 - not used
#define GSM_RXD   PIN2_bm      /* PD2 - вход, данные из NB-IoT модуля BC66 */
#define GSM_TXD   PIN3_bm      /* PD3 - выход, данные в NB-IoT модуль BC66 */
// PD4 - PD7 - not used

// PORTE pins:
// PE0, PE1 - not used
// PE2 - TOSC2 (32768Hz)
// PE3 - TOSC1 (32768Hz)

// PORTR pins:
// PR0 - XTAL2 (not used)
// PR1 - XTAL1 (not used)

#define GSM_port  PORTD         /* порт для GSM USART */
#define GSM_usart USARTD0       /* USART для GSM */

#define GSM_PWR_port     PORTA  /* ключ VT1, VT2 (управляет подачей питания на модуль M66, BC66) */
#define GSM_PWR_ON_port  PORTB  /* ключ VT3 (управляет PWRKEY модуля M66, BC66) */

#define I2C_port	PORTC
#define I2C_module	&TWIC

#endif /* CB_BOARD_H_ */
