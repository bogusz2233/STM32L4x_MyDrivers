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
#include "leds.h"
#include "buzzer.h"

uint32_t seconds = 0;
uint64_t milliSeconds = 0;

/*
 * 	Init miliseconds timer
 */
inline void timerMsInit(void)
{
	timerMsClockEnable();

	TIMER_MS_PERH->CNT = 0;	//clear count reg

	//base timer configuration
	TIMER_MS_PERH->PSC |=	TIMER_MS_PRESCALER;
	TIMER_MS_PERH->ARR = 1000;				//Irq every 1000 ms

	TIMER_MS_PERH->CR1 &= ~ TIM_CR1_CKD;
	TIMER_MS_PERH->CR1 |= TIMER_MS_CKD << TIM_CR1_CKD_Pos;

	TIMER_MS_PERH->DIER |= TIM_DIER_UIE;	//update irqu enable

	NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 2);
	NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

	TIMER_MS_PERH->CR1 |= TIM_CR1_CEN; // timer start count
}


uint32_t timerGetMsTime(void)
{
	return ((uint32_t)TIMER_MS_PERH->CNT + seconds * 1000);
}

/*
 * 	Init microseconds timer
 */
void timerUsInit(void)
{
	timerUsClockEnable();
	TIMER_US_PERH->CNT = 0;

	//base timer configuration
	TIMER_US_PERH->PSC |=	TIMER_US_PRESCALER;
	TIMER_US_PERH->ARR = 1000;				//Irq every 1000 us

	TIMER_US_PERH->CR1 &= ~ TIM_CR1_CKD;
	TIMER_US_PERH->CR1 |= TIMER_US_CKD << TIM_CR1_CKD_Pos;

	TIMER_US_PERH->DIER |= TIM_DIER_UIE;	//update irqu enable

	TIMER_US_PERH->CR1 |= TIM_CR1_CEN;
}

uint64_t timerGetUsTime(void)
{
	return ((uint64_t)TIMER_US_PERH->CNT + milliSeconds * 1000);
}

void timerDelayUs(uint64_t delayUsTime)
{
	uint64_t startDelayTime = timerGetUsTime();

	//Wait until delayUsTime pass
	while( (timerGetUsTime() - startDelayTime) > delayUsTime);
}

void TIM1_UP_TIM16_IRQHandler(void)
{
	if(TIMER_MS_PERH->SR & TIM_SR_UIF)
	{
		TIMER_MS_PERH->CNT = 0;
		TIMER_MS_PERH->SR &= ~ TIM_SR_UIF;
		seconds++;
	}
	if(TIMER_US_PERH->SR & TIM_SR_UIF)
	{
		TIMER_US_PERH->CNT = 0;
		TIMER_US_PERH->SR &= ~ TIM_SR_UIF;
		milliSeconds++;
	}

	timerMiliSecondService();
}

__attribute__((weak)) void timerMiliSecondService(void)
{
	//nothing here, you must overwrite it somewhere
}

#endif /* STM32L4X_MYDRIVER_TIMERSDRIVER_C_ */
