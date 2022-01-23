// 23.01.2022 удалил лишнее и закомментировал ненужное

// Based on code from:
// http://www.batsocks.co.uk/downloads/XMega_BleepingDemo_1-00.zip
// https://github.com/forkineye/PixelStick/blob/master/PixelStick/PixelStick.c
// http://forkineye.com/electronics/configure-avr-xmega-32mhz/

#include "syst_clock.h"

void init_SystemClock_Internal_DFLL( void )
{
	// ѕереключение на внутренний генератор 32ћ√ц, включение автокалибровки от внешнего кварца 32.768к√ц и получение 12ћ√ц
	
	// OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc | OSC_X32KLPM_bm; // выбор внешнего кварца 32.768к√ц и low-power режима дл€ него
	OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc;            // выбор внешнего кварца 32.768к√ц
	OSC.CTRL |= OSC_RC32MEN_bm | OSC_XOSCEN_bm;		// включить внутренний 32ћ√ц и внешний кварц
	while(!(OSC.STATUS & OSC_XOSCRDY_bm));			// подождать готовности внешнего кварца
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));         // подождать готовности внутреннего генератора 32ћ√ц
	OSC.DFLLCTRL = OSC_RC32MCREF_XOSC32K_gc;		// выбор внешнего кварца 32.768к√ц дл€ автокалибровки внутреннего генератора 32ћ√ц
	DFLLRC32M.CTRL = DFLL_ENABLE_bm ;               // Enable DFLL
	
	OSC.PLLCTRL = OSC_PLLSRC_RC32M_gc | 6 ;			// внутренний генератор 32ћ√ц деленный на 4 как источник дл€ PLL. ”множаем на 6, получаем 48ћ√ц.
	OSC.CTRL |= OSC_PLLEN_bm ;						// enable the PLL
	while(!(OSC.STATUS & OSC_PLLRDY_bm));			// подождать готовности PLL
	
	CCP = CCP_IOREG_gc;								// Disable register security for clock update
	CLK.PSCTRL = 0x0C;								// 00001100b делитель ј=4, делитель¬=делитель—=1, частота долна быть 48/4=12ћ√ц
	
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
	OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc | OSC_X32KLPM_bm; // выбор внешнего кварца 32.768к√ц и low-power режима дл€ него
	OSC.CTRL |= OSC_XOSCEN_bm;                            // включить внешний кварц 32.768к√ц
	while(!(OSC.STATUS & OSC_XOSCRDY_bm));			      // подождать готовности внешнего кварца
	CCP = CCP_IOREG_gc;                                   // Disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;					      // The System clock is now the external oscillator output (32.768 к√ц дл€ платы klz-v2-board))
	OSC.CTRL &= ~OSC_RC2MEN_bm;                           // Disable 2Mhz oscillator
}
*/
