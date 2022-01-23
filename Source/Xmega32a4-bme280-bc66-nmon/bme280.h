// Changed: 23.01.2022

// Based on code from:
// https://www.avrfreaks.net/forum/try-understand-compiler-logic-about-static-varsfuncs

#ifndef _BME280_H
#define _BME280_H

//#include <stdint.h>
#include "board3.h"


/* registers addresses */
#define BME280_ID_ADDR                      0xD0
#define BME280_RESET_ADDR                   0xE0
#define BME280_TP_CALIB_START_ADDR          0x88
#define BME280_HUM_H1_CALIB_START_ADDR      0xA1
#define BME280_HUM_REST_CALIB_START_ADDR    0xE1
#define BME280_CTRL_HUM_ADDR                0xF2
#define BME280_CTRL_MEAS_ADDR               0xF4
#define BME280_STATUS_ADDR                  0xF3
#define BME280_TPH_START_ADDR               0xF7
#define BME280_PRESS_START_ADDR             0xF7
#define BME280_TEMP_START_ADDR              0xFA
#define BME280_HUM_START_ADDR               0xFD
// config for t_standby, IIR filter, SPI enable
#define BME280_CFG_ADDR						0xF5

/* lengths of data registers */
#define BME280_TP_CALIB_LENGTH          24
#define BME280_H_CALIB_LENGTH           8
#define BME280_HUM_REST_CALIB_LENGTH    7
#define BME280_TPH_LENGTH               8
#define BME280_PRESS_LENGTH             3
#define BME280_TEMP_LENGTH              3
#define BME280_HUM_LENGTH               2

/* some influental parameters */
#define BME280_ID           0x60
#define BME280_I2C_ADDR     0x76

/* available commands and corresponding codes */
#define BME280_RESET_CMD    0xB6

/* limits for raw data */
#define BME280_TEMP_LOW     -4000
#define BME280_TEMP_HIGH    8500
#define BME280_PRESS_LOW    30000
#define BME280_PRESS_HIGH   110000
#define BME280_HUM_LOW      0
#define BME280_HUM_HIGH     99000

/* error codes */
#define BME280_ERR_NO			0
#define BME280_ERR_CONN			1
#define BME280_ERR_ID			2
#define BME280_ERR_CALIB		3
#define BME280_ERR_ACQ			4
#define BME280_ERR_STATUS		5
#define BME280_ERR_IO			6
#define BME280_ERR_FORCED_MODE	7 // 26.11.2020

// добавил:
// оверсамплинг
#define OVER_0x					0x00    // не измерять T, P или H
#define OVER_1x					0x01    // измерять без оверсэмплинга
#define OVER_2x					0x02    // измерять с оверсэмплингом 2
#define OVER_4x					0x03    // измерять с оверсэмплингом 4
#define OVER_8x					0x04    // измерять с оверсэмплингом 8
#define OVER_16x				0x05    // измерять с оверсэмплингом 16
// режим измерений
#define BME280_FORCED_MODE		0x01    // однократный запуск
#define BME280_NORMAL_MODE		0x03    // циклическая работа, время задается в битах t_sb[2:0] регистра config (0xF5)
#define BME280_SLEEP_MODE		0x00    // сон
// standbay time, t_sb[2:0], задается в битах 7,6,5 регистра config (0xF5)
#define BME280_STANDBY_500us	0x00
#define BME280_STANDBY_62500us	0x01
#define BME280_STANDBY_125ms	0x02
#define BME280_STANDBY_250ms	0x03
#define BME280_STANDBY_500ms	0x04
#define BME280_STANDBY_1000ms	0x05
#define BME280_STANDBY_10ms		0x06
#define BME280_STANDBY_20ms		0x07
// IIR filter, filter[2:0], задается в битах 4,3,2 регистра config (0xF5)
#define BME280_IIR_OFF			0x00   // выключен
#define BME280_IIR_2x			0x01   // коэффициент фильтрации 2
#define BME280_IIR_4x			0x02   // коэффициент фильтрации 4
#define BME280_IIR_8x			0x03   // коэффициент фильтрации 8
#define BME280_IIR_16x			0x04   // коэффициент фильтрации 16
// spi3w_en[0], выбор интерфейса I2C или SPI, задается в бите 0 регистра config (0xF5)
#define BME280_SPI_OFF			0x00
#define BME280_SPI_ON			0x01


uint8_t BME280_read_id(unsigned char *id);
uint8_t BME280_reset(void);
uint8_t BME280_read_calibration(void);

uint8_t BME280_set_acquisition(const unsigned char os_t, const unsigned char os_p, const unsigned char os_h, const unsigned char mode, \
const unsigned char t_sb, const unsigned char iir_filter, const unsigned char spi3w_en);

uint8_t BME280_read_status(unsigned char *status);
uint8_t BME280_read_TPH(void);
void    BME280_compensate(int16_t *final_T, uint32_t *final_P, uint32_t *final_H);

// add:
uint8_t BME280_forced_mode_start(const unsigned char os_t, const unsigned char os_p, const unsigned char os_h, const unsigned char mode);

// changed:
/* get readings from sensor */
extern unsigned char BME280_measure(int16_t *T, uint32_t *P, uint32_t *H);

#endif
