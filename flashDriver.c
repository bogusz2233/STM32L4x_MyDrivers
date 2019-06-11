/*
 * flashDriver.c
 *
 *  Created on: Jun 7, 2019
 *      Author: lukasz
 */

//private define
#define FLASH_UNLOCK_KEY1		0x45670123
#define FLASH_UNLOCK_KEY2		0xCDEF89AB

//private function
static void unlockFlash(void);
static void lockFlash(void);
static void checkAndClearProgramErrors(void);

#include "main.h"
#include "flashDriver.h"

/*
 * 	Unlocking flash, alow to write and other flash operation
 */
static void unlockFlash(void)
{
	FLASH->KEYR = FLASH_UNLOCK_KEY1;
	FLASH->KEYR = FLASH_UNLOCK_KEY2;
}

/*
 * 	Lock flash, protect against unwanted flash operation
 */
static void lockFlash(void)
{
	//cr lock flag
	while(FLASH->SR & FLASH_SR_BSY);	//wait until flash free
	FLASH->CR |= FLASH_CR_LOCK;
	while(!(FLASH->CR & FLASH_CR_LOCK));	//wait until flash free
}

/*
 * 	Page erase function, it erase 2KB flash
 * 		pageAdress - page adress to erase
 */
void flashDriverPageErase(uint8_t pageAdress)
{

	if(FLASH->CR & FLASH_CR_LOCK)
	{
		//if flash is locked, unlock this
		unlockFlash();
	}

	while(FLASH->SR & FLASH_SR_BSY);	//wait until flash operation is availble

	//check and clear programming error flag
	checkAndClearProgramErrors();

	FLASH->CR |= FLASH_CR_PER;	//Page erase enable

	//set page to erase
	FLASH->CR &= ~FLASH_CR_PNB;
	FLASH->CR |= pageAdress << FLASH_CR_PNB_Pos;

	FLASH->CR |= FLASH_CR_STRT;	//start erasing
	while(FLASH->SR & FLASH_SR_BSY);	//wait until flash erased

	FLASH->CR &= ~FLASH_CR_PER;	//Page erase disable

	lockFlash();

}

/*
 * 	Check if there are any program error, handler them
 */
static void checkAndClearProgramErrors(void)
{

	if(FLASH->SR & FLASH_SR_PGSERR)
	{
		if(FLASH->SR & FLASH_SR_FASTERR)
		{
			FLASH->SR |= FLASH_SR_FASTERR;
		}

		if(FLASH->SR & FLASH_SR_MISERR)
		{
			FLASH->SR |= FLASH_SR_MISERR;
		}

		if(FLASH->SR & FLASH_SR_SIZERR)
		{
			FLASH->SR |= FLASH_SR_SIZERR;
		}

		if(FLASH->SR & FLASH_SR_PGAERR)
		{
			FLASH->SR |= FLASH_SR_PGAERR;
		}

		if(FLASH->SR & FLASH_SR_WRPERR)
		{
				FLASH->SR |= FLASH_SR_WRPERR;
		}

		if(FLASH->SR & FLASH_SR_PROGERR)
		{
			FLASH->SR |= FLASH_SR_PROGERR;
		}

		FLASH->SR |= FLASH_SR_PGSERR;
	}

}

void flashDriverProramDoubleWord(uint32_t adress, uint64_t Data)
{
	while(FLASH->SR & FLASH_SR_BSY);	//wait until flash operation is availble

	if(FLASH->CR & FLASH_CR_LOCK)
	{
		//if flash is locked, unlock this
		unlockFlash();
	}

	checkAndClearProgramErrors();

	FLASH->CR |= FLASH_CR_PG;	//start program
	* (__IO uint32_t *) adress = (uint32_t) Data;
	* (__IO uint32_t *)  (adress + 4U) = (uint32_t) (Data >> 32);

	while(FLASH->SR & FLASH_SR_BSY);	//wait until program

	if(FLASH->SR & FLASH_SR_EOP)
	{
		//program sucessfull
	}
	else
	{
		//program error
	}

	FLASH->CR &= ~FLASH_CR_PG;	//end program
	lockFlash();
}

