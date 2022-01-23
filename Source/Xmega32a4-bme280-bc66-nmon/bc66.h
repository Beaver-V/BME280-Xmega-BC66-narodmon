// Changed: 23.01.2022

#ifndef BC66_H
#define BC66_H

#include "board3.h"

/* вектор прерывания по приему байта из модуля BC66 */
#define GSM_usart_RXC_vect USARTD0_RXC_vect

// буфер для приема  данных из модема
#define GSM_RX_BUF_size 256

char GSM_RX_buffer[GSM_RX_BUF_size];

volatile unsigned char rx_char_cnt;
volatile unsigned char GSM_received_byte;

char pwr_on_BC66(void);						// Включение - вывод PWRKEY=0 на 550 мс
void pwr_off_BC66(void);					// Выключение - командой AT+QPOWD=0

void clear_GSM_RX_buffer(void);             // очистка буфера приема  данных из модема
char wait_for(char *find, char timeout);    // 1s поиск строки в приемном буфере
char wait_for20(char *find, char timeout);  // 20ms поиск строки в приемном буфере

void init_GSM_usart(void);
void close_GSM_usart(void);
void GSM_usart_putc(char c);
void GSM_usart_putdata(char *data);
char sync_BC66_UART(void);

void red_led_blink(uint8_t blink_cnt);		// красный светодиод для индикации ошибок

char wait_for_pwr_on(char timeout);
char wait_for_pwr_off(char timeout);

char at_qsclk(void);										// отключение сна UART
char at_cpin(void);											// проверка наличия SIM карты
// char at_qcgdefcont(void);								// узнаем контекст
// char at_cgdcont (void);									// установка контекста
// void at_cgmr(void);										// версия прошивки BC66
char at_cereg(void);										// проверка регистрации в NB-IoT сети
char at_cbc(void);											// напряжение батарейки
char at_qeng_BC66(void);									// информация о базовой станции

char at_qiopen(char *my_server, char *my_port);				// команда подключения по TCP к серверу, Direct push mode
char at_qiopen_3tries(char *my_server, char *my_port);		// 3 попытки подключится к серверу
// char at_qird(void);										// чтение сообщения сервера - в Direct push mode они выводятся сами
char at_qiclose(void);										// отключение от сервера

// отправка данных на сервер
char at_qisend_var3(char *report_text_len, char *report_text);	// AT+QISEND=<connectID>,<send_length>
char at_qisend_CR_LF(void);										// AT+QISENDEX отправка 0x0D 0x0A

void GSM_usart_ISR_on(void);  // разрешение прерываний от USART
void GSM_usart_ISR_off(void); // запрещение прерываний от USART

#endif /* MODEM_H */