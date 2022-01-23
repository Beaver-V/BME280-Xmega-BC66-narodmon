// Changed: 23.01.2022
// Давление - в гектопаскалях (было в мм рт столба)

// Based on code from:
// https://www.avrfreaks.net/forum/try-understand-compiler-logic-about-static-varsfuncs

#include "bme280.h"
//#include "twi.h"

#include <stdint.h>

/* calibration coefficients */
typedef struct {
    uint16_t T1;
    int16_t T2;
    int16_t T3;

    uint16_t P1;
    int16_t P2;
    int16_t P3;
    int16_t P4;
    int16_t P5;
    int16_t P6;
    int16_t P7;
    int16_t P8;
    int16_t P9;

    unsigned char H1;
    int16_t H2;
    unsigned char H3;
    int16_t H4;
    int16_t H5;
    char H6;
} BME280_coeffs_t;

/* raw sensor data */
typedef struct {
    unsigned char t0;
    unsigned char t1;
    unsigned char t2;
    unsigned char p0;
    unsigned char p1;
    unsigned char p2;
    unsigned char h0;
    unsigned char h1;
} BME280_data_t;

BME280_coeffs_t BME280_calib;
BME280_data_t BME280_tph;


/* read sensor ID */
uint8_t BME280_read_id(unsigned char *id) {
    
	*id = 0;

	if (i2c_start(I2C_module, BME280_I2C_ADDR, 0))    // RW=0 (write)
	    return 0;

	if (i2c_write(I2C_module, BME280_ID_ADDR))        // BME280 ID register address
	    return 0;
	
	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 1))  // RW=1 (read). Restart to change the operation type from write to read
	    return 0;

	*id = i2c_read(I2C_module, I2C_NACK);             // read BME280 ID

	i2c_stop(I2C_module);                             // stop
	
    return 1;
}


/* issue reset */
uint8_t BME280_reset(void) {
	
	if (i2c_start(I2C_module, BME280_I2C_ADDR, 0))              // RW=0 (write)
	return 0;

	if (i2c_write(I2C_module, BME280_RESET_ADDR))               // BME280 reset register address
	return 0;

	if (i2c_write(I2C_module, BME280_RESET_CMD))                // BME280 reset command
	return 0;

	i2c_stop(I2C_module);                                       // stop

	return 1;
}

/* read calibration coefficients */
uint8_t BME280_read_calibration(void) {
	
	unsigned char tp_coeffs[BME280_TP_CALIB_LENGTH];
	unsigned char h1_coeff;
	unsigned char h_coeffs[BME280_HUM_REST_CALIB_LENGTH];
	
	/* read T and P calibration coefficients */
	
	if (i2c_start(I2C_module, BME280_I2C_ADDR, 0))                 // старт, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_TP_CALIB_START_ADDR))         // начальный адрес калибровочных байтов TP
	return 0;

	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 1))               // рестарт для смены типа операции с записи на чтение, адрес датчика + RW=1 (чтение)
	return 0;

	i2c_read_nbytes(tp_coeffs, BME280_TP_CALIB_LENGTH);

	/* read H1 calibration coefficient */
	
	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 0))               // рестарт для смены адреса калибровочного байта, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_HUM_H1_CALIB_START_ADDR))     // адрес калибровочного байта HUM_H1
	return 0;

	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 1))               // рестарт для смены типа операции с записи на чтение, адрес датчика + RW=1
	return 0;

	h1_coeff = i2c_read(I2C_module, I2C_NACK);                     // чтение калибровочного байта HUM_H1

	/* read H2-H6 calibration coefficients */
	
	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 0))               // рестарт для смены адреса калибровочного байта, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_HUM_REST_CALIB_START_ADDR))   // начальный адрес калибровочных байтов HUM_REST
	return 0;

	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 1))               // рестарт для смены типа операции с записи на чтение, адрес датчика + RW=1
	return 0;

	i2c_read_nbytes(h_coeffs, BME280_HUM_REST_CALIB_LENGTH);

	i2c_stop(I2C_module);

	/* properly combine bytes to get numeric coefficients */
	
	BME280_calib.T1 = (((uint16_t)tp_coeffs[1]) << 8) | ((uint16_t)tp_coeffs[0]);
	BME280_calib.T2 = (((int16_t)tp_coeffs[3]) << 8) | ((int16_t)tp_coeffs[2]);
	BME280_calib.T3 = (((int16_t)tp_coeffs[5]) << 8) | ((int16_t)tp_coeffs[4]);

	BME280_calib.P1 = (((uint16_t)tp_coeffs[7]) << 8) | ((uint16_t)tp_coeffs[6]);
	BME280_calib.P2 = (((int16_t)tp_coeffs[9]) << 8) | ((int16_t)tp_coeffs[8]);
	BME280_calib.P3 = (((int16_t)tp_coeffs[11]) << 8) | ((int16_t)tp_coeffs[10]);
	BME280_calib.P4 = (((int16_t)tp_coeffs[13]) << 8) | ((int16_t)tp_coeffs[12]);
	BME280_calib.P5 = (((int16_t)tp_coeffs[15]) << 8) | ((int16_t)tp_coeffs[14]);
	BME280_calib.P6 = (((int16_t)tp_coeffs[17]) << 8) | ((int16_t)tp_coeffs[16]);
	BME280_calib.P7 = (((int16_t)tp_coeffs[19]) << 8) | ((int16_t)tp_coeffs[18]);
	BME280_calib.P8 = (((int16_t)tp_coeffs[21]) << 8) | ((int16_t)tp_coeffs[20]);
	BME280_calib.P9 = (((int16_t)tp_coeffs[23]) << 8) | ((int16_t)tp_coeffs[22]);

	BME280_calib.H1 = (unsigned char)h1_coeff;
	BME280_calib.H2 = (((int16_t)h_coeffs[1]) << 8) | ((int16_t)h_coeffs[0]);
	BME280_calib.H3 = (unsigned char)h_coeffs[2];
	BME280_calib.H4 = (((int16_t)h_coeffs[3]) << 4) | ((int16_t)(h_coeffs[4] & 0x0F));
	BME280_calib.H5 = (((int16_t)h_coeffs[5]) << 4) | ((int16_t)(h_coeffs[4] >> 4));
	BME280_calib.H6 = (char)h_coeffs[6];

	return 1;
}

/* set measurement parameters */
uint8_t BME280_set_acquisition(const unsigned char os_t, const unsigned char os_p, const unsigned char os_h, const unsigned char mode, \
const unsigned char t_sb, const unsigned char iir_filter, const unsigned char spi3w_en) {
	
	/* write parameters for humidity oversampling first */
	
	if (i2c_start(I2C_module, BME280_I2C_ADDR, 0))                   // старт, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_CTRL_HUM_ADDR))                 // адрес регистра CTRL для влажности
	return 0;

	if (i2c_write(I2C_module, os_h))                                 // os_h=1 - измерять влажность без оверсамплинга
	return 0;

	/* then write the rest */
	
	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 0))                 // рестарт для смены адреса записи, адрес датчика + RW=0 (запись)
	return 0;
	if (i2c_write(I2C_module, BME280_CFG_ADDR))                      // адрес регистра фильтра IIR
	return 0;
	if (i2c_write(I2C_module, ((t_sb << 5) | (iir_filter << 2) | spi3w_en)))
	return 0;

	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 0))                 // рестарт для смены адреса записи, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_CTRL_MEAS_ADDR))                // адрес регистра оверсамплинга температуры, давления и режима работы BME280
	return 0;

	if (i2c_write(I2C_module, ((os_t << 5) | (os_p << 2) | mode)))   // os_t=1, os_p=1 - температура, давление без оверсамплинга
	return 0;                                                        // mode=1         - однократный запуск
	
	i2c_stop(I2C_module);

	return 1;
}

uint8_t BME280_forced_mode_start(const unsigned char os_t, const unsigned char os_p, const unsigned char os_h, const unsigned char mode) {
	
	/* write parameters for humidity oversampling first */
	
	if (i2c_start(I2C_module, BME280_I2C_ADDR, 0))     // - старт, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_CTRL_HUM_ADDR))   // адрес регистра CTRL для влажности
	return 0;

	if (i2c_write(I2C_module, os_h))                   // os_h=1 - измерять влажность без оверсамплинга
	return 0;

	/* then write the rest */
	
	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 0))   // рестарт для смены адреса записи, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_CTRL_MEAS_ADDR))  // адрес регистра оверсамплинга температуры, давления и режима работы BME280
	return 0;

	if (i2c_write(I2C_module, ((os_t << 5) | (os_p << 2) | mode))) // os_t=1, os_p=1 - температура, давление без оверсамплинга.
	return 0;                                                 // mode=1 - однократный запуск
	
	i2c_stop(I2C_module);

	return 1;
}

/* read sensor status */
uint8_t BME280_read_status(unsigned char *status) {
	
	unsigned char s;

	if (i2c_start(I2C_module, BME280_I2C_ADDR, 0))    // старт, адрес датчика + RW=0 (запись)
	return 0;

	if (i2c_write(I2C_module, BME280_STATUS_ADDR))
	return 0;

	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 1))
	return 0;

	s = i2c_read(I2C_module, I2C_NACK);               // &s, *s, s

	i2c_stop(I2C_module);

	*status = s & 0x09;

	return 1;
}

/* read raw measurements */
uint8_t BME280_read_TPH(void) {
	
	unsigned char data[BME280_TPH_LENGTH];

	if (i2c_start(I2C_module, BME280_I2C_ADDR, 0))
	return 0;

	if (i2c_write(I2C_module, BME280_TPH_START_ADDR))
	return 0;

	if (i2c_restart(I2C_module, BME280_I2C_ADDR, 1))
	return 0;

	i2c_read_nbytes(data, BME280_TPH_LENGTH);

	i2c_stop(I2C_module);

	BME280_tph.t0 = data[3];
	BME280_tph.t1 = data[4];
	BME280_tph.t2 = data[5];

	BME280_tph.p0 = data[0];
	BME280_tph.p1 = data[1];
	BME280_tph.p2 = data[2];

	BME280_tph.h0 = data[6];
	BME280_tph.h1 = data[7];

	return 1;
}

/* convert raw measurements data into physical values */
void BME280_compensate(int16_t *final_T, uint32_t *final_P, uint32_t *final_H) {
	
	int32_t t_raw = (int32_t)((((int32_t)BME280_tph.t0) << 12) | (((int32_t)BME280_tph.t1) << 4) | (((int32_t)BME280_tph.t2) >> 4));
	int32_t var1, var2, t_fine, T;
	var1 = (((t_raw >> 3) - (((int32_t)BME280_calib.T1) << 1)) * ((int32_t)BME280_calib.T2)) >> 11;
	var2 = (((((t_raw >> 4) - ((int32_t)BME280_calib.T1)) * ((t_raw >> 4) - ((int32_t)BME280_calib.T1))) >> 12) * ((int32_t)BME280_calib.T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;

	if ((T <= BME280_TEMP_LOW) || (T >= BME280_TEMP_HIGH)) {
		if (T <= BME280_TEMP_LOW)
		*final_T = BME280_TEMP_LOW;
		else
		*final_T = BME280_TEMP_HIGH;
	}
	else {
		*final_T = T;   // температура в сотых долях: 4000=40.00 градусов Цельсия
	}

	int32_t p_raw = (int32_t)((((uint32_t)BME280_tph.p0) << 12) | (((uint32_t)BME280_tph.p1) << 4) | (((uint32_t)BME280_tph.p2) >> 4));
	uint32_t P;
	var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)BME280_calib.P6);
	var2 = var2 + ((var1 * ((int32_t)BME280_calib.P5)) << 1);
	var2 = (var2 >> 2) + (((int32_t)BME280_calib.P4) << 16);
	var1 = (((BME280_calib.P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)BME280_calib.P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t)BME280_calib.P1)) >> 15);
	P = (((uint32_t)(((int32_t)1048576) - p_raw) - (var2 >> 12))) * 3125;

	if (var1 != 0) {
		if (P < 0x80000000)
		P = (P << 1) / ((uint32_t)var1);
		else
		P = (P / ((uint32_t)var1)) * 2;

		var1 = (((int32_t)BME280_calib.P9) * ((int32_t) (((P >> 3) * (P >> 3)) >> 13))) >> 12;
		var2 = (((int32_t)(P >> 2)) * ((int32_t)BME280_calib.P8)) >> 13;
		P = (uint32_t)((int32_t)P + ((var1 + var2 + BME280_calib.P7) >> 4));
	}
	else
	P = 0;

	if ((P <= BME280_PRESS_LOW) || (P >= BME280_PRESS_HIGH)) {
		if (P < BME280_PRESS_LOW)
		// *final_P = BME280_PRESS_LOW / 133.322;               // давление в мм рт ст
		*final_P = BME280_PRESS_LOW;							// в гектопаскалях
		else
		// *final_P = BME280_PRESS_HIGH / 133.322;              // давление в мм рт ст
		*final_P = BME280_PRESS_HIGH;							// в гектопаскалях
	}
	else {
		// *final_P = P / 133.322;                              // давление в мм рт ст
		*final_P = P;											// в гектопаскалях
	}

	int32_t h_raw = (int32_t)((((int32_t)BME280_tph.h0) << 8) | ((int32_t)BME280_tph.h1));
	int32_t var3, var4, var5;
	uint32_t H;
	var1 = (int32_t)t_fine - (int32_t)76800;
	var2 = h_raw << 14;
	var3 = ((int32_t)BME280_calib.H4) << 20;
	var4 = ((int32_t)BME280_calib.H5) * var1;
	var5 = ((var2 - var3 - var4 + (int32_t)16384) >> 15);
	var2 = (var1 * ((int32_t)BME280_calib.H6)) >> 10;
	var3 = (var1 * ((int32_t)BME280_calib.H3)) >> 11;
	var4 = ((var2 * (var3 + (int32_t)32768)) >> 10) + (int32_t)2097152;
	var2 = (var4 * ((int32_t)BME280_calib.H2) + 8192) >> 14;
	var3 = var5 * var2;
	var4 = ((var3 >> 15) * (var3 >> 15)) >> 7;
	var5 = var3 - ((var4 * ((int32_t)BME280_calib.H1)) >> 4);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);
	H = (uint32_t)(var5 >> 12);

	if ((H <= BME280_HUM_LOW) || (H >= BME280_HUM_HIGH)) {
		if (H <= BME280_HUM_LOW)
		*final_H = BME280_HUM_LOW;
		else
		*final_H = BME280_HUM_HIGH / 1000; // влажность в процентах
	}
	else {
		*final_H = H / 1000;               // влажность в процентах
	}
}

/* perform one measurement cycle and get results */
unsigned char BME280_measure(int16_t *T, uint32_t *P, uint32_t *H) {
	
	unsigned char id;

	if (!BME280_read_id(&id))
	return BME280_ERR_CONN;

	if (id != BME280_ID)
	return BME280_ERR_ID;

	if (!BME280_read_calibration())
	return BME280_ERR_CALIB;

	if (!BME280_forced_mode_start(OVER_1x, OVER_1x, OVER_1x, BME280_FORCED_MODE))
	return BME280_ERR_FORCED_MODE;
	
	/* wait until sensor becomes idle */
	unsigned char status;
	do {
		if (!BME280_read_status(&status))
		return BME280_ERR_STATUS;
	} while (status);

	if (status == 0) {
		if (!BME280_read_TPH())
		return BME280_ERR_IO;

		BME280_compensate(T, P, H); /* perform conversion */

		return BME280_ERR_NO;
	 }
	 else
	 return BME280_ERR_STATUS;
}
