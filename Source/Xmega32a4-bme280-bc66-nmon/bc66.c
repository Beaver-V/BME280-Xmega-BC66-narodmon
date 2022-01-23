// Changed: 23.01.2022

#include "bc66.h"

// BC66 power on
char pwr_on_BC66(void)
{
	GSM_PWR_port.OUTCLR = LED1;			// green LED on
	GSM_PWR_ON_port.OUTSET = GSM_ON;	// вывести единицу для включения (PWRKEY=0)
	_delay_ms(550);						// для BC66 задержка >= 500 мс
	GSM_PWR_ON_port.OUTCLR = GSM_ON;	// вывести ноль для освобождения PWRKEY
	GSM_PWR_port.OUTSET = LED1;			// green LED off
	
	// ждать включения GSM модуля, определяем по наличию VDD_EXT на входе GSM_STAT
	if (wait_for_pwr_on(10)) // 10x100ms=1s
		return 1;
	else
		return 0;
}

// BC66 power off
void pwr_off_BC66(void)
{
	GSM_usart_putdata("AT+QPOWD=0\r");
	wait_for_pwr_off(250);				// до 250x100ms=25s ждем пропадания VDD_EXT
}

// очистка буфера приема  данных из модема
void clear_GSM_RX_buffer(void)
{
	unsigned short i=0;
	for(i=0; i<=GSM_RX_BUF_size; i++) GSM_RX_buffer[i]=0;
	rx_char_cnt=0;
}


// поиск строки в приемном буфере 1 секунда минимум
char wait_for(char * find,char timeout)
{
	while(timeout--)
	{
		_delay_ms(1000);
		if(strstr(GSM_RX_buffer, find)) return 1;
	};
	return 0;
};

// поиск строки в приемном буфере 20 мс минимум
char wait_for20(char *find, char timeout)
{
	while(timeout--)
	{
		_delay_ms(20);
		if(strstr(GSM_RX_buffer,find)) return 1;
	};
	return 0;
};

void init_GSM_usart(void)
{
	GSM_port.DIRSET = GSM_TXD ;			// TXD as output
	GSM_port.OUTSET = GSM_TXD ;			// TXD high
	GSM_port.DIRCLR = GSM_RXD ;			// RXD as input
	
	// CTRLA - уровень прерывания для приема, передачи и опустошению регистра данных; 00 - прерывания запрещены
	// разрешены прерывания от приемника и передатчика:
	//GSM_usart.CTRLA = USART_RXCINTLVL0_bm | USART_TXCINTLVL0_bm;
	// разрешены прерывания только от приемника:
	GSM_usart.CTRLA = USART_RXCINTLVL0_bm ;
	
	// для 9600, разрешение работы приемника и передатчика:
	//GSM_usart.CTRLB = USART_RXEN_bm | USART_TXEN_bm ;
	// для 115200, разрешение работы приемника и передатчика и удвоение скорости:
	GSM_usart.CTRLB = USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm;
	
	//  асинхронный USART, без четности, 1 стоп-бит, 8 бит размер символа:
	GSM_usart.CTRLC = 3 ;
	
	// 115200
	GSM_usart.BAUDCTRLA = (((F_CPU) / (8)) / 115200) - 1;
	
	// 9600
	// GSM_usart.BAUDCTRLA = (((F_CPU) / (16)) / 9600) - 1 ;
	
	GSM_usart.BAUDCTRLB = 0 ;
	
	// прерывания разрешаются в main.c
	// PMIC.CTRL = PMIC_LOLVLEN_bm;
}

void close_GSM_usart(void)
{
	GSM_port.DIRSET = GSM_TXD ;	// TXD as output
	GSM_port.OUTCLR = GSM_TXD ; // TXD low
	GSM_port.DIRSET = GSM_RXD ; // RXD as output
	GSM_port.OUTCLR = GSM_RXD ; // RXD low
	GSM_usart.CTRLA = 0x00 ;    // 00 - прерывания запрещены
	GSM_usart.CTRLB = 0 ;       // запрешение работы приемника и передатчика
	GSM_usart.CTRLC = 0 ;       //
	GSM_usart.BAUDCTRLA = 0 ;   //
	GSM_usart.BAUDCTRLB = 0 ;
}

void GSM_usart_putc(char c)
{
	while (!(GSM_usart.STATUS & USART_DREIF_bm));
	_delay_us(1);
	GSM_usart.DATA = c;
}

void GSM_usart_putdata(char *data)
{
	while (*data != '\0')
	{
		GSM_usart_putc(*data);
		data++;
	}
}

char sync_BC66_UART(void)
{
	uint8_t a;
	for (uint8_t i=0; i<5; i++)
	{
		GSM_usart_putdata("AT\r");
		if (wait_for20("OK", 15))	// 20*15=300ms
		{
			a = 1;
			break;
		}
		else a = 0;
	}
	return a;
}

void red_led_blink(uint8_t blink_cnt)
{
	for (uint8_t i = 0 ; i < blink_cnt ; i++)
	{
		GSM_PWR_port.OUTCLR = LED2;      // red LED on
		_delay_ms(200);
		GSM_PWR_port.OUTSET = LED2;      // red LED off
		_delay_ms(200);
	}
}

char wait_for_pwr_on(char timeout)
{
	while(timeout--)
	{
		_delay_ms(100);
		if((GSM_PWR_ON_port.IN) & GSM_STAT) return 1;		// GSM_STAT=VDD_EXT=1.8V - модуль включен
	}
	return 0;	// GSM_STAT=VDD_EXT=0V - модуль выключен
}

char wait_for_pwr_off(char timeout)
{
	while(timeout--)
	{
		_delay_ms(100);
		if(!((GSM_PWR_ON_port.IN) & GSM_STAT)) return 1;	// GSM_STAT=VDD_EXT=0V - модуль выключен
	}
	return 0;	// GSM_STAT=VDD_EXT=1.8V - модуль включен
}

// BC66 отключение сна
char at_qsclk(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QSCLK=0\r");
	if (wait_for20("OK", 15))	// 20*15=300ms
		return 1;
	else
		return 0;
}

// проверка наличия SIM карты
char at_cpin(void)
{
	uint8_t a;
	for (uint8_t i=0; i<5; i++)
	{
		clear_GSM_RX_buffer();
		GSM_usart_putdata("AT+CPIN?\r");
		if (wait_for20("CPIN: READY", 25)) // 20ms*25=500ms and 500ms*5=2500ms
		{
			a = 1;
			break;
		}
		else a = 0;
	}
	return a;
}

// узнаем дефолтный контекст
/*
char at_qcgdefcont(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QCGDEFCONT?\r");
	_delay_ms(100);
	return 1;
}
*/

// BC66 установка контекста
/*
char at_cgdcont (void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CGDCONT=1,");
	GSM_usart_putc(0x22); // кавычки
	GSM_usart_putdata("IP");
	GSM_usart_putc(0x22); // кавычки
	GSM_usart_putdata(",");
	GSM_usart_putc(0x22);
	GSM_usart_putdata("internet.mts.ru");  // APN для МТС
	GSM_usart_putc(0x22);
	// для BC66 логин и пароль не нужны, если отправлять, ответит ERROR
	// GSM_usart_putdata(",");
	// GSM_usart_putc(0x22);
	// GSM_usart_putdata("mts");  // логин для МТС
	// GSM_usart_putc(0x22);
	// GSM_usart_putdata(",");
	// GSM_usart_putc(0x22);
	// GSM_usart_putdata("mts");  // пароль для МТС
	// GSM_usart_putc(0x22);
	GSM_usart_putdata("\r");
	_delay_ms(100);
	return 1;
}
*/

// версия прошивки BC66 и UUID, IMEI, IMEISV, SVN
/*
void at_cgmr(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CGSN=0\r");
	_delay_ms(100);
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CGSN=1\r");
	_delay_ms(100);
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CGSN=2\r");
	_delay_ms(100);
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CGSN=3\r");
	_delay_ms(100);
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CGMR\r");
	_delay_ms(100);
}
*/

// BC66 - проверка регистрации в NB-IoT сети
char at_cereg(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CEREG?\r");
	_delay_ms(80);
	if (wait_for20("+CEREG: 0,1",46)) // 20мс*46=920мс и 920+80=1000мс=1 секунда
	return 1;
	else return 0;
}

// напряжение батарейки
char at_cbc(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+CBC\r");
	_delay_ms(100);
	return 1;
}

// BC66 информация о базовой станции
char at_qeng_BC66(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QENG=0\r");
	_delay_ms(100);
	return 1;
}

// соединимся по TCP с сервером
// AT+QIOPEN=<contextID>,<connectID>,<service_type>,<IP_address or domain_name>,<remote_port>
// [,<local_port>[,<access_mode>][,<protocol_type>]]
char at_qiopen(char *my_server, char *my_port)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QIOPEN=1,0,");
	GSM_usart_putc(0x22);			// "
	GSM_usart_putdata("TCP");
	GSM_usart_putc(0x22);			// "
	GSM_usart_putdata(",");
	GSM_usart_putc(0x22);			// "
	GSM_usart_putdata(my_server);
	GSM_usart_putc(0x22);			// "
	GSM_usart_putdata(",");
	GSM_usart_putc(0x22);			// "
	GSM_usart_putdata(my_port);
	GSM_usart_putc(0x22);			// "
	// GSM_usart_putdata(",0,0\r");	// local_port=0, Buffer access mode
	GSM_usart_putdata(",0,1\r");	// local_port=0, Direct push mode

	if (wait_for20("OK", 15))	// 20*15=300ms
		return 1;
	else
		return 0;
}

char at_qiopen_3tries(char *my_server, char *my_port)
{
	uint8_t a = 0;
	for (uint8_t i=0; i<3; i++)						// 3 попытки подключится к серверу
	{
		at_qiopen(my_server, my_port);				// команда at+qiopen
		if (wait_for("+QIOPEN: 0,0", 45))			// до 45 секунд ожидание подключения к серверу
		{
			// подключились
			a = 1;
			break;
		}
		else if (!at_qiclose())						// останов неудачной попытки подключится
		{
			// ошибка или таймаут при выполнении команды at+qiclose
			a = 0;
			break;
		}
		// получен CLOSE OK, повторим попытку
	}
	// или подключились, или закончились попытки или ошибка
	return a;
}

// чтение сообщения сервера - в Direct push mode они выводятся сами
/*
char at_qird(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QIRD=0,512\r");
	_delay_ms(100);
	return 1;
}
*/

// отключение от сервера
char at_qiclose(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QICLOSE=0\r");
	if (wait_for20("CLOSE OK", 250)) // 20*250=5000ms
	{
		return 1;
	}
	else return 0;
}

// отправка сообщения на сервер - вариант с приглашением и длиной
// AT+QISEND=<connectID>,<send_length>
char at_qisend_var3(char *report_text_len, char *report_text)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QISEND=0,");
	GSM_usart_putdata(report_text_len);
	GSM_usart_putdata("\r");
	if (wait_for20(">",25))				// 20*25=500ms
	{
		GSM_usart_putdata(report_text);
		if (wait_for20("SEND OK",250))	// 20*250=5000ms
		{
			return 1;
		}
		else return 0;
	}
	else return 0;
}

// отправка 0x0D, 0x0A на сервер
char at_qisend_CR_LF(void)
{
	clear_GSM_RX_buffer();
	GSM_usart_putdata("AT+QISENDEX=0,2,0D0A\r");
	_delay_ms(100);
	return 1;
}

// разрешение прерываний от USART
void GSM_usart_ISR_on(void)
{
	// разрешены прерывания только от приемника
	GSM_usart.CTRLA = USART_RXCINTLVL0_bm ;
}

// запрещение прерываний от USART
void GSM_usart_ISR_off(void)
{
	GSM_usart.CTRLA = 0x00 ; // 00 - прерывания запрещены
}

//прерывание по приему байта из GSM USART
ISR(GSM_usart_RXC_vect)
{
	GSM_received_byte = GSM_usart.DATA;
	GSM_RX_buffer[rx_char_cnt++] = GSM_received_byte;
	if (rx_char_cnt > GSM_RX_BUF_size)
	{
		rx_char_cnt = 0;
	}
}
