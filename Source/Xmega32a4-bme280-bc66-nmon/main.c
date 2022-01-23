/*
 * Xmega32a4-bme280-bc66-nmon.c
 * Created: 02.02.2021
 * Changed: 23.01.2022
 * Author : Beaver-V
 */ 

// Based on code by Odyssey Nabiev
// Thank you Odys!

// �������� ��������, ����������� � ��������� �� ���� narodmon.com
// ATXMEGA32A4U           - ���������������
// BC66NB-04-STD          - ������� ������ (NB-IoT Cat NB1)
// BME280                 - ������ ��������, ����������� � ���������


#include "board3.h"

// ��������� ������ ���������
#define START_DEV		0
#define GET_VBAT_RSSI	1
#define WAIT_IP			2
#define SEND_REPORT		3
#define GO_TO_SLEEP		4
#define ERROR_PWR_ON	10
#define ERROR_BATTERY	11
#define ERROR_WAIT_IP	12
#define ERROR_CBC		13
#define ERROR_QENG		14
#define ERROR_QIOPEN	15
#define ERROR_QISEND	16


#define WORK_PERIOD		599	// ������� ������ - �������� ����� ���������� �� ������.
#define CEREG_ATTEMPTS	3	// ����� ������� ������������������ � ����
#define QIOPEN_ATTEMPTS	3	// ����� ������� ����������� � �������

uint8_t state;				// ������� ��������� ������ ���������
uint8_t cereg_count;		// ������� ������� ������������������ � ����
uint8_t qiopen_count;		// ������� ������� ���������� � �������� QIOPEN attempts counter

int16_t  T;					// �����������
uint32_t P;					// ��������
uint32_t H;					// ���������

char T_for_print[5];		// ����������� ��� �������� �� ������
char T_for_print_tmp[5];	// ��� �������������� ����������� � ascii
char P_for_print[7];		// �������� ��� �������� �� ������
char H_for_print[5];		// ��������� ��� �������� �� ������

char V_batt[5];				// ���������� ��������� ��� �������� �� ������
char RSSI_level[5];			// ������� ������� ���� ��� �������� �� ������
char report_text[70];
char *report_text_len;

uint8_t quotient_part_T;	// ��� �������������� ����������� � ascii
uint8_t remainder_part_T;	// ��� �������������� ����������� � ascii


// Narodmon
char* nmon_mac_id = "#Your-narodmon-ID";
char* my_server   = "narodmon.ru";
char* my_port     = "8283";


ISR(RTC_COMP_vect)
{
	// �������� RTC ���������� � ������� 3 �������� ������ "������ RTC". F_RTC=1kHz => 3ms
	while(RTC_STATUS & RTC_SYNCBUSY_bm);
	RTC_CNT=0;
};


int main(void)
{
	// ������������� ������������ MCU - 12���
	init_SystemClock_Internal_DFLL();
	
	// LED init
	GSM_PWR_port.DIRSET = LED1;      // output for green LED
	GSM_PWR_port.DIRSET = LED2;      // output for red LED
	GSM_PWR_port.OUTCLR = LED1;      // green LED on
	GSM_PWR_port.OUTTGL = LED2;      // red LED on
	
	// GSM_STAT - ����
	GSM_PWR_ON_port.DIRCLR = GSM_STAT;
	
	// GSM_ON - �����
	GSM_PWR_ON_port.DIRSET = GSM_ON;	// ������� �������
	GSM_PWR_ON_port.OUTCLR = GSM_ON;	// ������� ���� ��� ������������ PWRKEY
	
	// ������������� TWI
	// ��������� ���������� �������� (AVR1308-Using-the-XMEGA-TWI).
	PORTCFG.MPCMASK = 0x03;				// Configure several PINxCTRL registers at the same time
	I2C_port.PIN0CTRL = (PORTC.PIN0CTRL & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc;	//Enable pull-up
	i2c_init(I2C_module, TWI_BAUD(F_CPU, F_TWI));								//  BAUD_100K
	
	// ����� BME280
	BME280_reset();
	_delay_ms(3);	// ?
	
	// ������������� BME280
	BME280_set_acquisition(OVER_1x, OVER_1x, OVER_1x, BME280_FORCED_MODE, BME280_STANDBY_500us, BME280_IIR_OFF, BME280_SPI_OFF);
	
	state = START_DEV;
	cereg_count = CEREG_ATTEMPTS;		// ���������� ������� ������������������ � ����
	qiopen_count = QIOPEN_ATTEMPTS;		// ���������� ������� ���������� � ��������
	
	// ���������� ����������
	PMIC.CTRL = PMIC_LOLVLEN_bm;	// ��������� ���������� ������� ������
	sei();
	
	// ���������� ��� ����������������
	set_sleep_mode(SLEEP_SMODE_PSAVE_gc);
	sleep_enable();
	
	// ������������� RTC
	// ���� RTC ��� �� �������, �� ����� ������������� ����� ������� � �������� �� ����������� (��. AVR1314)
	
	// 1. ��������� ������������ RTC � ����� ������������ �� �������� �������� ������ 32.768��� �������� �� 1024
	// CLK_RTCSRC_TOSC_gc - 1.024 kHz from 32.768 kHz crystal oscillator on TOSC
	CLK_RTCCTRL = CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm;
	
	// 2. Write period value by writing to the PER register.
	// ����� ����� �������� PER ���� ������ �������� COMP, ����� �� ����� ���������� �� ���������� �� RTC.
	RTC.PER = 0xFFFF;
	
	// 3. ��������� ���������� �������� �������� (��������������� � � ���������� RTC)
	RTC_CNT = 0;
	
	// 4. �������� ������������ ��������� ����� ���������� �� ������ � ������� ���������
	RTC_COMP = WORK_PERIOD;
	
	// 5. ���������� ���������� �� RTC �� ����������, ��������� ������
	RTC_INTCTRL = ( RTC.INTCTRL & ~RTC_COMPINTLVL_gm ) | RTC_COMPINTLVL_LO_gc;
	
	// 6. ������ �������� RTC
	// ��������� ������������ ��������� �������. �������� �������� ������� �� 1024 (PRESCALER[2:0]=7=111b)
	RTC_CTRL =  ( RTC_CTRL & ~RTC_PRESCALER_gm ) | RTC_PRESCALER_DIV1024_gc;
	
	// green and red LEDs off
	_delay_ms(200);
	GSM_PWR_port.OUTSET = LED1;      // ��� ����������� ������� � ����� ��� ���������� ���������� �� 200 ��
	GSM_PWR_port.OUTSET = LED2;
	
	while(1)
	{
			
		switch(state)
		{
			
			case START_DEV:
			
			// Green and red LED off, ���� ��� ���� �������� � ���������� ����� ��� ��������� ������.
			GSM_PWR_port.OUTSET = LED1;
			GSM_PWR_port.OUTSET = LED2;
			
			// ���������� �����������, ��������, ���������
			BME280_measure(&T, &P, &H);
			
			// ����������� ��� �������
			memset(T_for_print, 0x00, sizeof(T_for_print));
			if (T < 0x0000)
			{
				T = abs(T);                                // ���������� ��������: abs(-1234)=1234
				strcat (T_for_print, "-");                 // ���� ����� ��� ������������� ����������
			}
			else
			{
				// strcat (T_for_print, "+");              // ���� ���� ��� ������������� ����������
			}
			//
			// ����� ����� �����������
			quotient_part_T = (T / 100);
			memset(T_for_print_tmp, 0x00, sizeof(T_for_print_tmp));
			utoa(quotient_part_T, T_for_print_tmp, 10);
			strcat (T_for_print, T_for_print_tmp);
			// ����������� ����� � ������� ������ (�����)
			strcat (T_for_print, ".");
			// ������� ����� �����������
			remainder_part_T = (T % 100);
			if (remainder_part_T < 10)
			{
				strcat (T_for_print, "0");
			}
			memset(T_for_print_tmp, 0x00, sizeof(T_for_print_tmp));
			utoa(remainder_part_T, T_for_print_tmp, 10);
			strcat (T_for_print, T_for_print_tmp);
			
			// �������� ��� �������
			memset(P_for_print, 0x00, sizeof(P_for_print));
			ultoa(P, P_for_print, 10);
			
			// ��������� ��� �������
			memset(H_for_print, 0x00, sizeof(H_for_print));
			ultoa(H, H_for_print, 10);
			
			// ��������� BC66
			if (!pwr_on_BC66())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// ���������, ��������� USART XMEGA
			init_GSM_usart();					// 115200
			clear_GSM_RX_buffer();
			
			
			// ������� AT � ������ ��� ������������� �������� UART BC66
			if (!sync_BC66_UART())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// ���������� ��� BC66
			if (!at_qsclk())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// �������� ������� SIM �����
			if (!at_cpin())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// ������ �������� QCGDEFCONT
			// at_qcgdefcont();
						
			// ���� ��������� ���, ����� ���������
			
			// ������ �������� BC66
			// at_cgmr();
			
			state = WAIT_IP;
			break; // ��� case START_DEV
			
			
			case WAIT_IP:
			
			// �������� ����������� � ����
			
			for (uint8_t i=0; i<45; i++)        // 45 ������
			{
				GSM_PWR_port.OUTTGL = LED1;     // green LED toggle
				if (at_cereg())
				{
					state=GET_VBAT_RSSI;        // ������������������
					GSM_PWR_port.OUTCLR = LED1; // green LED on
					break;
				}
				else
				{
					if (wait_for_pwr_off(1)) // ��������, �� ���������� �� ������?
					{
						state=ERROR_BATTERY; // ������ ���������� ��� ����������� � ���� (���� ���������).
						break;
					}
					state=ERROR_WAIT_IP;       // �� ������������������
				}
			}
			
			cereg_count--;
			
			if ((state==ERROR_WAIT_IP) && (cereg_count > 0))
			{
				pwr_off_BC66();			// �� 25 ������ �������� ���������� BC66
				state = START_DEV;		// ����������� ������������������ ��� ���
				break;
			}
			
			cereg_count = CEREG_ATTEMPTS;	// ����� ������� ������������������ � ���� ����� ����������
			
			break; // ��� case WAIT_IP
			
			
			case GET_VBAT_RSSI:
			
			// �������� ��� ������������ ���������� ����� �����������
			// ��� ������ ������� (������� �����) ����������� �� ��������
			_delay_ms(1000);
			
			// battery voltage AT+CBC<CR> Answer: CR><LF>+CBC: 0,0,3280<CR><LF><CR><LF>OK<CR><LF>
			at_cbc();
			memset(V_batt, 0x00, sizeof(V_batt));
			
			if (wait_for("OK",1))
			{
				GSM_usart_ISR_off();	// ���������� ���������� �� USART
				
				// ���������, ������� ����
				char str2[] = "CBC:";
				
				// ��������� �� ������ ���������� � ����������
				char *istr = NULL;
				
				// �����
				istr = strstr (GSM_RX_buffer , str2);
				
				if ( istr == NULL)
				{
					// ������, ������� VBAT=1111 (1.111 �����)
					memset(V_batt, 0x31, (sizeof(V_batt) - 1));
				}
				else
				{
					for (uint8_t i=0 ; i < 4 ; i++)
					{
						// ������ ���������� ���������
						V_batt[i] = *(istr + 9 + i);
					}
				}
				GSM_usart_ISR_on(); // ���������� ���������� �� USART
			}
			
			// ���������� � ����
			// engineering mode AT+QENG=0<CR>
			// <CR><LF>+QENG: 0,1839,2,481,"35E5D4",-85,-6,-78,5,3,"4B1F",0,,0<CR><LF><CR><LF>OK<CR><LF>
			at_qeng_BC66();
			memset(RSSI_level, 0x00, sizeof(RSSI_level));
			
			if (wait_for20("OK",150)) // ���� ����� OK
			{
				// �������� ����������, ��� �� � �������� ����� ������ ������ �� ����������
				GSM_usart_ISR_off(); // ���������� ���������� �� USART
				
				// �������� ������ �������� ������ ������� RSSI
				uint8_t comma_count = 0;					// ������� �������
				uint8_t rssi_level_index = 0;				// ������ ��� RSSI_level[]
				for (uint8_t i = 0 ; i < rx_char_cnt ; i++)
				{
					if (GSM_RX_buffer[i] == ',')
					{
						comma_count++;
						if (comma_count == 7)				// ������� �������, �� ��� ������ ���� �������� RSSI
						{
							i++;
							while (GSM_RX_buffer[(i)] != ',')
							{
								RSSI_level[rssi_level_index] = GSM_RX_buffer[(i)];
								rssi_level_index++;
								i++;
							}
						}
					}
				}
				// ��������� RSSI
				// �������� ����������
				GSM_usart_ISR_on(); // ���������� ���������� �� USART
				
				// ����� ������� ��� narodmon
				memset (report_text, 0x00, sizeof(report_text));
				//
				strcat (report_text, nmon_mac_id);
				strcat (report_text, "\n");
				//
				strcat (report_text, "#T1#");
				strcat (report_text, T_for_print);
				strcat (report_text, "\n");
				//
				strcat (report_text, "#P1#");
				strcat (report_text, P_for_print);
				strcat (report_text, "\n");
				//
				strcat (report_text, "#H1#");
				strcat (report_text, H_for_print);
				strcat (report_text, "\n");
				//
				strcat (report_text, "#U1#");
				strcat (report_text, V_batt);
				strcat (report_text, "\n");
				//
				strcat (report_text, "#RSSI#");
				strcat (report_text, RSSI_level);
				strcat (report_text, "\n");
				//
				strcat (report_text, "##");
				//
				// ������ �������
				utoa ((strlen(report_text)), report_text_len, 10);
				
				state=SEND_REPORT;
				break;
			}
			else
			{
				state=ERROR_QENG;
				break;
			}

			break;
			
			
			case SEND_REPORT:
			
			// ���������� �� TCP � ��������
			if (!at_qiopen_3tries(my_server, my_port))
			{
				// �� ������������ �� 3 �������
				state = ERROR_QIOPEN;
				break;
			}
			
			// ������������ � �������, �������� ������
			if (!at_qisend_var3(report_text_len, report_text))
			{
				// ������ ��� �������� ������, ������� �����
				// at_qiclose();
				state = ERROR_QISEND;
				break;
			}
			
			// ������ ���������, ��� ������������� ����� �� ������� ��������
			// � Direct push mode �������� �� ������� ������ ����� ��������� � UART
			clear_GSM_RX_buffer();      // ����� �������� OK ������ �� �������, � �� �� ����������� SEND OK
			if (!wait_for("OK" , 10))	// 1s*10=10s
			{
				// ������ �� ���������� ���� ������, ������� �����
				// at_qiclose();
				state = ERROR_QISEND;
				break;
			}
			
			// ������ ���������� ���� ������, ������� �����
			at_qiclose();
			state = GO_TO_SLEEP;
			
			// ���������� ������� ���������� � �������� ����� ������ �� ���
			qiopen_count = QIOPEN_ATTEMPTS; // 3
			
			// ������� ������� ���������
			GSM_PWR_port.OUTSET = LED1;
			
			break;

			
			case ERROR_BATTERY:
			red_led_blink(1);	// 5
			
			case ERROR_PWR_ON:
			red_led_blink(1);	// 4
			
			case ERROR_WAIT_IP:
			red_led_blink(1);	// 3
			
			case ERROR_CBC:
			red_led_blink(1);	// 2
			
			case ERROR_QENG:
			red_led_blink(1);	// 1
			
			state=GO_TO_SLEEP;
			break;
			
			
			case ERROR_QISEND:
			GSM_PWR_port.OUTCLR = LED1;	// green LED on
			red_led_blink(1);			// 2
			
			case ERROR_QIOPEN:
			red_led_blink(1);			// 1
			
			GSM_PWR_port.OUTSET = LED1;	// green LED off
			at_qiclose();
			state = GO_TO_SLEEP;
			break;
			
			
			case GO_TO_SLEEP:
			
			pwr_off_BC66();
			close_GSM_usart();
			
			// ����� ��������� ������������� ��������� RTC � ������ ����� ����� ������� � ��� (��. AVR1314)
			// �������� RTC ���������� � ������� 3 �������� ������ "������ RTC". F_RTC=1kHz => 3ms
			while(RTC_STATUS & RTC_SYNCBUSY_bm);
			
			sleep_cpu();
			
			// Exit from sleep by the RTC interrupt.
			state=START_DEV;
			break;
			
			default:
			break;
			
		}   // end switch
	}       // end while(1)
}           // end main()
