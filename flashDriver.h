/*
 * flashDriver.h
 *
 *  Created on: Jun 7, 2019
 *      Author: lukasz
 */

#ifndef STM32L4X_MYDRIVER_FLASHDRIVER_H_
#define STM32L4X_MYDRIVER_FLASHDRIVER_H_

void flashDriverPageErase(uint8_t);
void flashDriverProramDoubleWord(uint32_t, uint64_t);

#endif /* STM32L4X_MYDRIVER_FLASHDRIVER_H_ */
