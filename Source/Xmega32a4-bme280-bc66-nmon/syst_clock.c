// 23.01.2022 ������ ������ � ��������������� ��������

// Based on code from:
// http://www.batsocks.co.uk/downloads/XMega_BleepingDemo_1-00.zip
// https://github.com/forkineye/PixelStick/blob/master/PixelStick/PixelStick.c
// http://forkineye.com/electronics/configure-avr-xmega-32mhz/

#include "syst_clock.h"

void init_SystemClock_Internal_DFLL( void )
{
	// ������������ �� ���������� ��������� 32���, ��������� �������������� �� �������� ������ 32.768��� � ��������� 12���
	
	// OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc | OSC_X32KLPM_bm; // ����� �������� ������ 32.768��� � low-power ������ ��� ����
	OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc;            // ����� �������� ������ 32.768���
	OSC.CTRL |= OSC_RC32MEN_bm | OSC_XOSCEN_bm;		// �������� ���������� 32��� � ������� �����
	while(!(OSC.STATUS & OSC_XOSCRDY_bm));			// ��������� ���������� �������� ������
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));         // ��������� ���������� ����������� ���������� 32���
	OSC.DFLLCTRL = OSC_RC32MCREF_XOSC32K_gc;		// ����� �������� ������ 32.768��� ��� �������������� ����������� ���������� 32���
	DFLLRC32M.CTRL = DFLL_ENABLE_bm ;               // Enable DFLL
	
	OSC.PLLCTRL = OSC_PLLSRC_RC32M_gc | 6 ;			// ���������� ��������� 32��� �������� �� 4 ��� �������� ��� PLL. �������� �� 6, �������� 48���.
	OSC.CTRL |= OSC_PLLEN_bm ;						// enable the PLL
	while(!(OSC.STATUS & OSC_PLLRDY_bm));			// ��������� ���������� PLL
	
	CCP = CCP_IOREG_gc;								// Disable register security for clock update
	CLK.PSCTRL = 0x0C;								// 00001100b �������� �=4, ���������=���������=1, ������� ����� ���� 48/4=12���
	
	CCP = CCP_IOREG_gc;                             // Disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_PLL_gc;					// The System clock is now the PLL output (((32MhzRC/4)*6)/4)
	OSC.CTRL &= ~OSC_RC2MEN_bm;                     // Disable 2Mhz oscillator
}

/*
void init_SystemClock_Internal_2M (void)
{
	OSC.CTRL |= OSC_RC2MEN_bm;					// 2Mhz internal oscillator
	while(!(OSC.STATUS & OSC_RC2MEN_bm));
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_RC2M_gc;
}
*/

/*
void init_SystemClock_Internal_32M (void)
{
	OSC.CTRL |= OSC_RC32MEN_bm;					// 32Mhz internal oscillator
	while(!(OSC.STATUS & OSC_RC32MEN_bm));
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
}
*/

/*
void init_SystemClock_External( void )
{
	OSC.XOSCCTRL = OSC_FRQRANGE_2TO9_gc | OSC_XOSCSEL_XTAL_16KCLK_gc ;
	OSC.CTRL |= OSC_XOSCEN_bm ;
	while( (OSC.STATUS & OSC_XOSCRDY_bm) == 0 ){} // wait until stable
	// switch to the external oscillator as the clocksource
	CCP = CCP_IOREG_gc; // protected write follows
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;
}
*/

/*
void init_SystemClock_32K_External( void )
{
	OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc | OSC_X32KLPM_bm; // ����� �������� ������ 32.768��� � low-power ������ ��� ����
	OSC.CTRL |= OSC_XOSCEN_bm;                            // �������� ������� ����� 32.768���
	while(!(OSC.STATUS & OSC_XOSCRDY_bm));			      // ��������� ���������� �������� ������
	CCP = CCP_IOREG_gc;                                   // Disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;					      // The System clock is now the external oscillator output (32.768 ��� ��� ����� klz-v2-board))
	OSC.CTRL &= ~OSC_RC2MEN_bm;                           // Disable 2Mhz oscillator
}
*/
