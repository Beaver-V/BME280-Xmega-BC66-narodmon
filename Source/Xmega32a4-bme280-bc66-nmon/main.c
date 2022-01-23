/*
 * Xmega32a4-bme280-bc66-nmon.c
 * Created: 02.02.2021
 * Changed: 23.01.2022
 * Author : Beaver-V
 */ 

// Based on code by Odyssey Nabiev
// Thank you Odys!

// Передача давления, температуры и влажности на сайт narodmon.com
// ATXMEGA32A4U           - микроконтроллер
// BC66NB-04-STD          - сотовый модуль (NB-IoT Cat NB1)
// BME280                 - датчик давления, температуры и влажности


#include "board3.h"

// Состояния машины состояний
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


#define WORK_PERIOD		599	// рабочий период - интервал между отправками на сервер.
#define CEREG_ATTEMPTS	3	// число попыток зарегистрироваться в сети
#define QIOPEN_ATTEMPTS	3	// число попыток подключится к серверу

uint8_t state;				// Текущее состояние машины состояний
uint8_t cereg_count;		// Счётчик попыток зарегистрироваться в сети
uint8_t qiopen_count;		// Счётчик попыток соединится с сервером QIOPEN attempts counter

int16_t  T;					// температура
uint32_t P;					// давление
uint32_t H;					// влажность

char T_for_print[5];		// температура для отправки на сервер
char T_for_print_tmp[5];	// для преобразования температуры в ascii
char P_for_print[7];		// давление для отправки на сервер
char H_for_print[5];		// влажность для отправки на сервер

char V_batt[5];				// напряжение батарейки для отправки на сервер
char RSSI_level[5];			// уровень сигнала сети для отправки на сервер
char report_text[70];
char *report_text_len;

uint8_t quotient_part_T;	// для преобразования температуры в ascii
uint8_t remainder_part_T;	// для преобразования температуры в ascii


// Narodmon
char* nmon_mac_id = "#Your-narodmon-ID";
char* my_server   = "narodmon.ru";
char* my_port     = "8283";


ISR(RTC_COMP_vect)
{
	// регистры RTC недоступны в течении 3 тактовых циклов "домена RTC". F_RTC=1kHz => 3ms
	while(RTC_STATUS & RTC_SYNCBUSY_bm);
	RTC_CNT=0;
};


int main(void)
{
	// инициализация тактирования MCU - 12МГц
	init_SystemClock_Internal_DFLL();
	
	// LED init
	GSM_PWR_port.DIRSET = LED1;      // output for green LED
	GSM_PWR_port.DIRSET = LED2;      // output for red LED
	GSM_PWR_port.OUTCLR = LED1;      // green LED on
	GSM_PWR_port.OUTTGL = LED2;      // red LED on
	
	// GSM_STAT - вход
	GSM_PWR_ON_port.DIRCLR = GSM_STAT;
	
	// GSM_ON - выход
	GSM_PWR_ON_port.DIRSET = GSM_ON;	// сделать выходом
	GSM_PWR_ON_port.OUTCLR = GSM_ON;	// вывести ноль для освобождения PWRKEY
	
	// инициализация TWI
	// Включение внутренней подтяжки (AVR1308-Using-the-XMEGA-TWI).
	PORTCFG.MPCMASK = 0x03;				// Configure several PINxCTRL registers at the same time
	I2C_port.PIN0CTRL = (PORTC.PIN0CTRL & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc;	//Enable pull-up
	i2c_init(I2C_module, TWI_BAUD(F_CPU, F_TWI));								//  BAUD_100K
	
	// сброс BME280
	BME280_reset();
	_delay_ms(3);	// ?
	
	// инициализация BME280
	BME280_set_acquisition(OVER_1x, OVER_1x, OVER_1x, BME280_FORCED_MODE, BME280_STANDBY_500us, BME280_IIR_OFF, BME280_SPI_OFF);
	
	state = START_DEV;
	cereg_count = CEREG_ATTEMPTS;		// Количество попыток зарегистрироваться в сети
	qiopen_count = QIOPEN_ATTEMPTS;		// Количество попыток соединится с сервером
	
	// разрешение прерываний
	PMIC.CTRL = PMIC_LOLVLEN_bm;	// разрешены прерывания низкого уровня
	sei();
	
	// разрешение сна микроконтроллера
	set_sleep_mode(SLEEP_SMODE_PSAVE_gc);
	sleep_enable();
	
	// инициализация RTC
	// Если RTC ещё не запущен, то ждать синхронизации перед записью в регистры не обязательно (см. AVR1314)
	
	// 1. Включение тактирования RTC и выбор тактирования от внешнего часового кварца 32.768кГц делённого на 1024
	// CLK_RTCSRC_TOSC_gc - 1.024 kHz from 32.768 kHz crystal oscillator on TOSC
	CLK_RTCCTRL = CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm;
	
	// 2. Write period value by writing to the PER register.
	// Важно чтобы значение PER было больше значения COMP, иначе не будет прерывания по совпадению от RTC.
	RTC.PER = 0xFFFF;
	
	// 3. Установка начального значения счетчика (устанавливается и в прерывании RTC)
	RTC_CNT = 0;
	
	// 4. Загрузка длительности интервала между отправками на сервер в регистр сравнения
	RTC_COMP = WORK_PERIOD;
	
	// 5. Разрешение прерывания от RTC по совпадению, приоритет низкий
	RTC_INTCTRL = ( RTC.INTCTRL & ~RTC_COMPINTLVL_gm ) | RTC_COMPINTLVL_LO_gc;
	
	// 6. запуск счетчика RTC
	// Установка предделителя запускает счетчик. Тактовые импульсы делятся на 1024 (PRESCALER[2:0]=7=111b)
	RTC_CTRL =  ( RTC_CTRL & ~RTC_PRESCALER_gm ) | RTC_PRESCALER_DIV1024_gc;
	
	// green and red LEDs off
	_delay_ms(200);
	GSM_PWR_port.OUTSET = LED1;      // при подключении питания к плате оба светодиода зажигаются на 200 мс
	GSM_PWR_port.OUTSET = LED2;
	
	while(1)
	{
			
		switch(state)
		{
			
			case START_DEV:
			
			// Green and red LED off, если они были включены в предыдущем цикле для индикации ошибки.
			GSM_PWR_port.OUTSET = LED1;
			GSM_PWR_port.OUTSET = LED2;
			
			// измеряются температура, давление, влажность
			BME280_measure(&T, &P, &H);
			
			// температура для репорта
			memset(T_for_print, 0x00, sizeof(T_for_print));
			if (T < 0x0000)
			{
				T = abs(T);                                // абсолютное значение: abs(-1234)=1234
				strcat (T_for_print, "-");                 // знак минус для отрицательных температур
			}
			else
			{
				// strcat (T_for_print, "+");              // знак плюс для положительных температур
			}
			//
			// целая часть температуры
			quotient_part_T = (T / 100);
			memset(T_for_print_tmp, 0x00, sizeof(T_for_print_tmp));
			utoa(quotient_part_T, T_for_print_tmp, 10);
			strcat (T_for_print, T_for_print_tmp);
			// разделитель целой и дробной частей (точка)
			strcat (T_for_print, ".");
			// дробная часть температуры
			remainder_part_T = (T % 100);
			if (remainder_part_T < 10)
			{
				strcat (T_for_print, "0");
			}
			memset(T_for_print_tmp, 0x00, sizeof(T_for_print_tmp));
			utoa(remainder_part_T, T_for_print_tmp, 10);
			strcat (T_for_print, T_for_print_tmp);
			
			// давление для репорта
			memset(P_for_print, 0x00, sizeof(P_for_print));
			ultoa(P, P_for_print, 10);
			
			// влажность для репорта
			memset(H_for_print, 0x00, sizeof(H_for_print));
			ultoa(H, H_for_print, 10);
			
			// включение BC66
			if (!pwr_on_BC66())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// включился, настройка USART XMEGA
			init_GSM_usart();					// 115200
			clear_GSM_RX_buffer();
			
			
			// выведем AT в модуль для синхронизации скорости UART BC66
			if (!sync_BC66_UART())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// отключение сна BC66
			if (!at_qsclk())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// проверка наличия SIM карты
			if (!at_cpin())
			{
				state = ERROR_PWR_ON;
				break;
			}
			
			// узнаем контекст QCGDEFCONT
			// at_qcgdefcont();
						
			// если контекста нет, нужно прописать
			
			// версия прошивки BC66
			// at_cgmr();
			
			state = WAIT_IP;
			break; // для case START_DEV
			
			
			case WAIT_IP:
			
			// ожидание регистрации в сети
			
			for (uint8_t i=0; i<45; i++)        // 45 секунд
			{
				GSM_PWR_port.OUTTGL = LED1;     // green LED toggle
				if (at_cereg())
				{
					state=GET_VBAT_RSSI;        // зарегистрировались
					GSM_PWR_port.OUTCLR = LED1; // green LED on
					break;
				}
				else
				{
					if (wait_for_pwr_off(1)) // проверка, не выключился ли модуль?
					{
						state=ERROR_BATTERY; // модуль выключился при регистрации в сети (села батарейка).
						break;
					}
					state=ERROR_WAIT_IP;       // не зарегистрировались
				}
			}
			
			cereg_count--;
			
			if ((state==ERROR_WAIT_IP) && (cereg_count > 0))
			{
				pwr_off_BC66();			// до 25 секунд ожидания выключения BC66
				state = START_DEV;		// попробовать зарегистрироваться ещё раз
				break;
			}
			
			cereg_count = CEREG_ATTEMPTS;	// число попыток зарегистрироваться в сети после просыпания
			
			break; // для case WAIT_IP
			
			
			case GET_VBAT_RSSI:
			
			// задержка для стабилизации напряжения после регистрации
			// при слабом сигнале (больших токах) практически не помогает
			_delay_ms(1000);
			
			// battery voltage AT+CBC<CR> Answer: CR><LF>+CBC: 0,0,3280<CR><LF><CR><LF>OK<CR><LF>
			at_cbc();
			memset(V_batt, 0x00, sizeof(V_batt));
			
			if (wait_for("OK",1))
			{
				GSM_usart_ISR_off();	// запрещение прерываний от USART
				
				// подстрока, которую ищем
				char str2[] = "CBC:";
				
				// указатель на начало совпадения с подстрокой
				char *istr = NULL;
				
				// поиск
				istr = strstr (GSM_RX_buffer , str2);
				
				if ( istr == NULL)
				{
					// Ошибка, сделаем VBAT=1111 (1.111 Вольт)
					memset(V_batt, 0x31, (sizeof(V_batt) - 1));
				}
				else
				{
					for (uint8_t i=0 ; i < 4 ; i++)
					{
						// читаем напряжение батарейки
						V_batt[i] = *(istr + 9 + i);
					}
				}
				GSM_usart_ISR_on(); // разрешение прерываний от USART
			}
			
			// информация о сети
			// engineering mode AT+QENG=0<CR>
			// <CR><LF>+QENG: 0,1839,2,481,"35E5D4",-85,-6,-78,5,3,"4B1F",0,,0<CR><LF><CR><LF>OK<CR><LF>
			at_qeng_BC66();
			memset(RSSI_level, 0x00, sizeof(RSSI_level));
			
			if (wait_for20("OK",150)) // надо ждать OK
			{
				// запретим прерывания, что бы в приемный буфер ничего больше не записалось
				GSM_usart_ISR_off(); // запрещение прерываний от USART
				
				// читается только значение уровня сигнала RSSI
				uint8_t comma_count = 0;					// счётчик запятых
				uint8_t rssi_level_index = 0;				// индекс для RSSI_level[]
				for (uint8_t i = 0 ; i < rx_char_cnt ; i++)
				{
					if (GSM_RX_buffer[i] == ',')
					{
						comma_count++;
						if (comma_count == 7)				// седьмая запятая, за ней должно быть значение RSSI
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
				// прочитали RSSI
				// разрешим прерывания
				GSM_usart_ISR_on(); // разрешение прерываний от USART
				
				// текст репорта для narodmon
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
				// размер репорта
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
			
			// соединимся по TCP с сервером
			if (!at_qiopen_3tries(my_server, my_port))
			{
				// не подключились за 3 попытки
				state = ERROR_QIOPEN;
				break;
			}
			
			// подключились к серверу, отправим данные
			if (!at_qisend_var3(report_text_len, report_text))
			{
				// ошибка при отправке данных, закроем сокет
				// at_qiclose();
				state = ERROR_QISEND;
				break;
			}
			
			// данные отправили, ждём подтверждения приёма от сервера народмон
			// в Direct push mode принятые от сервера данные сразу выводятся в UART
			clear_GSM_RX_buffer();      // чтобы получить OK именно от сервера, а не от предыдущего SEND OK
			if (!wait_for("OK" , 10))	// 1s*10=10s
			{
				// сервер не подтвердил приём данных, закроем сокет
				// at_qiclose();
				state = ERROR_QISEND;
				break;
			}
			
			// сервер подтвердил приём данных, закроем сокет
			at_qiclose();
			state = GO_TO_SLEEP;
			
			// Количество попыток соединится с сервером после выхода из сна
			qiopen_count = QIOPEN_ATTEMPTS; // 3
			
			// погасим зеленый светодиод
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
			
			// Нужно подождать синхронизации регистров RTC и только после этого уходить в сон (см. AVR1314)
			// регистры RTC недоступны в течении 3 тактовых циклов "домена RTC". F_RTC=1kHz => 3ms
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
