/*
 * timersDriver.h
 *
 *  Created on: Jun 10, 2019
 *      Author: lukasz
 */

#ifndef STM32L4X_MYDRIVER_TIMERSDRIVER_H_
#define STM32L4X_MYDRIVER_TIMERSDRIVER_H_

#define timerMsClockEnable()   RCC->APB2ENR |= RCC_APB2ENR_TIM1EN
#define TIMER_MS_PERH                  TIM1


#define TIMER_MS_CLOCK_DIV	1
#define TIMER_MS_PRESCALER 	((uint64_t)CLOCK_FREQ_MHZ * 1000000 *TIMER_MS_CLOCK_DIV/ 1000)

#if (TIMER_MS_CLOCK_DIV == 1)
	#define TIMER_MS_CKD 2
#elif (TIMER_MS_CLOCK_DIV == 2)
	#define TIMER_MS_CKD 1
#else
	#define TIMER_MS_CKD 0
#endif

#define TIMER_US_PERH			TIM16
#define timerUsClockEnable() RCC->APB2ENR |= RCC_APB2ENR_TIM16EN

#define TIMER_US_CLOCK_DIV	1
#define TIMER_US_PRESCALER 	((uint64_t) CLOCK_FREQ_MHZ * 1000000 * TIMER_US_CLOCK_DIV /  1000000)

#if (TIMER_US_CLOCK_DIV == 4)
	#define TIMER_US_CKD 2
#elif (TIMER_US_CLOCK_DIV == 2)
	#define TIMER_US_CKD 1
#else
	#define TIMER_US_CKD 0
#endif

//function
void timerMsInit(void);
uint32_t timerGetMsTime(void);

void timerUsInit(void);
uint64_t timerGetUsTime(void);
void timerDelayUs(uint64_t);

void TIM1_UP_TIM16_IRQHandler(void);
void timerMiliSecondService(void);


#endif /* STM32L4X_MYDRIVER_TIMERSDRIVER_H_ */
