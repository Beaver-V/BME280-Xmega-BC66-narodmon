// 23.01.2022 удалил лишнее и закомментировал ненужное

#ifndef SYST_CLOCK_H_
#define SYST_CLOCK_H_

#include <avr/io.h>

void init_SystemClock_Internal_DFLL( void );   // Внутренний осциллятор RC32M с автокалибровкой от внешнего кварца XOSC32K (32.768КГц)
// void init_SystemClock_Internal_32M (void);     // Внутренний осциллятор RC32M без автокалибровки
// void init_SystemClock_Internal_2M (void);      // Внутренний осциллятор RC2M без автокалибровки
// void init_SystemClock_External( void );        // Внешний кварц 12МГц
// void init_SystemClock_32K_External( void );    // Внешний кварц 32.768КГц

#endif /* SYST_CLOCK_H_ */