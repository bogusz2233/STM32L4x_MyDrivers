/*
 * timersDriver.c
 *
 *  Created on: Jun 10, 2019
 *      Author: lukasz
 */

#ifndef STM32L4X_MYDRIVER_TIMERSDRIVER_C_
#define STM32L4X_MYDRIVER_TIMERSDRIVER_C_

#include "main.h"
#include "timersDriver.h"

/*
 * 	Init miliseconds timer
 */
inline void timerMsInit(void)
{
	timerClockEnable();

	TIMER_PERH->CNT = 0;	//clear count reg

	//base timer configuration
	TIMER_PERH->CR1 |= TIM_CR1_ARPE;	//auto reload
	TIMER_PERH->CR1 |=	TIMER_MS_PRESCALER;

}

inline void timerStartCount(void)
{
	TIMER_PERH->CR1 |= TIM_CR1_CEN;
}

uint16_t timerGetTick(void)
{
	return TIMER_PERH->CNT;
}

#endif /* STM32L4X_MYDRIVER_TIMERSDRIVER_C_ */
