// 23.01.2022 ������ ������ � ��������������� ��������

#ifndef SYST_CLOCK_H_
#define SYST_CLOCK_H_

#include <avr/io.h>

void init_SystemClock_Internal_DFLL( void );   // ���������� ���������� RC32M � ��������������� �� �������� ������ XOSC32K (32.768���)
// void init_SystemClock_Internal_32M (void);     // ���������� ���������� RC32M ��� ��������������
// void init_SystemClock_Internal_2M (void);      // ���������� ���������� RC2M ��� ��������������
// void init_SystemClock_External( void );        // ������� ����� 12���
// void init_SystemClock_32K_External( void );    // ������� ����� 32.768���

#endif /* SYST_CLOCK_H_ */