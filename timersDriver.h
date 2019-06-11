/*
 * timersDriver.h
 *
 *  Created on: Jun 10, 2019
 *      Author: lukasz
 */

#ifndef STM32L4X_MYDRIVER_TIMERSDRIVER_H_
#define STM32L4X_MYDRIVER_TIMERSDRIVER_H_

#define timerClockEnable()  	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN
#define TIMER_PERH				TIM1

#define TIMER_MS_PRESCALER 	(CLOCK_FREQ_MHZ * 1000)

//function
void timerMsInit(void);
void timerStartCount(void);
uint16_t timerGetTick(void);

#endif /* STM32L4X_MYDRIVER_TIMERSDRIVER_H_ */
