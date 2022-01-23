// Changed: 23.01.2022

#ifndef BC66_H
#define BC66_H

#include "board3.h"

/* ������ ���������� �� ������ ����� �� ������ BC66 */
#define GSM_usart_RXC_vect USARTD0_RXC_vect

// ����� ��� ������  ������ �� ������
#define GSM_RX_BUF_size 256

char GSM_RX_buffer[GSM_RX_BUF_size];

volatile unsigned char rx_char_cnt;
volatile unsigned char GSM_received_byte;

char pwr_on_BC66(void);						// ��������� - ����� PWRKEY=0 �� 550 ��
void pwr_off_BC66(void);					// ���������� - �������� AT+QPOWD=0

void clear_GSM_RX_buffer(void);             // ������� ������ ������  ������ �� ������
char wait_for(char *find, char timeout);    // 1s ����� ������ � �������� ������
char wait_for20(char *find, char timeout);  // 20ms ����� ������ � �������� ������

void init_GSM_usart(void);
void close_GSM_usart(void);
void GSM_usart_putc(char c);
void GSM_usart_putdata(char *data);
char sync_BC66_UART(void);

void red_led_blink(uint8_t blink_cnt);		// ������� ��������� ��� ��������� ������

char wait_for_pwr_on(char timeout);
char wait_for_pwr_off(char timeout);

char at_qsclk(void);										// ���������� ��� UART
char at_cpin(void);											// �������� ������� SIM �����
// char at_qcgdefcont(void);								// ������ ��������
// char at_cgdcont (void);									// ��������� ���������
// void at_cgmr(void);										// ������ �������� BC66
char at_cereg(void);										// �������� ����������� � NB-IoT ����
char at_cbc(void);											// ���������� ���������
char at_qeng_BC66(void);									// ���������� � ������� �������

char at_qiopen(char *my_server, char *my_port);				// ������� ����������� �� TCP � �������, Direct push mode
char at_qiopen_3tries(char *my_server, char *my_port);		// 3 ������� ����������� � �������
// char at_qird(void);										// ������ ��������� ������� - � Direct push mode ��� ��������� ����
char at_qiclose(void);										// ���������� �� �������

// �������� ������ �� ������
char at_qisend_var3(char *report_text_len, char *report_text);	// AT+QISEND=<connectID>,<send_length>
char at_qisend_CR_LF(void);										// AT+QISENDEX �������� 0x0D 0x0A

void GSM_usart_ISR_on(void);  // ���������� ���������� �� USART
void GSM_usart_ISR_off(void); // ���������� ���������� �� USART

#endif /* MODEM_H */