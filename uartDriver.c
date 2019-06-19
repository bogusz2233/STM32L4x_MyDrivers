/*
 * uartDriver.c
 *
 *  Created on: Jun 4, 2019
 *      Author: lukasz
 */
#include "main.h"
#include "uartDriver.h"
#include "string.h"
#include "timersDriver.h"

//private function
static void uartGPIOInit(void);
static void uartSwitchToReceive(void);
static void uartSwitchToTransmit(void);
static void uartTransmit(uint8_t *, uint32_t);

UartStruct uartStruct;

/*
 *  Uart perh init
 */
void uartInit(void)
{
	uartGPIOInit();
	perhUartClockEnable();
	UART_PERH->BRR = USARTDIV_16_OVR & 0xffff;
	UART_PERH->CR1 |= USART_CR1_UE;

	//CR reg setup
	UART_PERH->CR1 |= 	USART_CR1_RXNEIE	//RXNEIE interrupt is enable
						|USART_CR1_RE		// Receiver mode enable
						|USART_CR1_TE;

	//set baudrate

	//interupt set
	NVIC_SetPriority(UART_PERH_IRQ, 3);
	NVIC_EnableIRQ(UART_PERH_IRQ);

	uartStruct.uartFreeFlag = UART_IS_FREE;		//uart is free
}

/*
 * Initialize uart port GPIO
 */
static inline void uartGPIOInit(void)
{
	portUartClockEnable();

	//Unlock port
	UART_TX_PORT->LCKR &= ~ GPIO_LCKR_LCK0 << UART_TX_PIN;
	UART_RX_PORT->LCKR &= ~ GPIO_LCKR_LCK0 << UART_RX_PIN;

	//set up MODER REG
	UART_TX_PORT->MODER &= ~(GPIO_MODER_MODE0_Msk << (UART_TX_PIN  * 2));	//clear mode flag
	UART_TX_PORT->MODER |= GPIO_MODER_MODE0_1 << (UART_TX_PIN * 2);		//alternate function

	UART_RX_PORT->MODER &= ~(GPIO_MODER_MODE0_Msk << (UART_RX_PIN * 2));	//clear mode flag
	UART_RX_PORT->MODER |= GPIO_MODER_MODE0_1 << (UART_RX_PIN * 2);		//alternate function

	//Setup OSPEEDR reg, speed work for pin
	UART_TX_PORT->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0_Msk << (UART_TX_PIN * 2));
	UART_TX_PORT->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0_1 << (UART_TX_PIN * 2);	// Set speed to high

	UART_RX_PORT->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0_Msk << (UART_RX_PIN * 2));
	UART_RX_PORT->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0_1 << (UART_RX_PIN * 2);
//
//	//set pull up
	UART_TX_PORT->PUPDR |= GPIO_PUPDR_PUPD0_0 << (UART_TX_PIN * 2);
	UART_RX_PORT->PUPDR |= GPIO_PUPDR_PUPD0_0 << (UART_RX_PIN * 2);

	//alternative active
	UART_TX_PORT->AFR[0] |= GPIO_AF7_USART3 << (UART_TX_PIN * 4);
	UART_RX_PORT->AFR[0] |= GPIO_AF7_USART3 << (UART_RX_PIN * 4);

}

void UART_PERH_IRQHandler(void)
{
	if((UART_PERH->ISR & USART_ISR_RXNE) && (UART_PERH->CR1 & USART_CR1_RE))
	{
		uartStruct.uartReveiveBuffer[uartStruct.countReceived ++] = UART_PERH->RDR;
	}
	if((UART_PERH->ISR & USART_ISR_TXE) && (UART_PERH->CR1 & USART_CR1_TE))
	{
		if(uartStruct.sizeToTransmit > 1)
		{
			UART_PERH->TDR = uartStruct.uartTransmitBuffer[uartStruct.countTransmit];
			uartStruct.countTransmit ++;
			uartStruct.sizeToTransmit--;
		}
		else
		{
			UART_PERH->TDR = uartStruct.uartTransmitBuffer[uartStruct.countTransmit];
			uartStruct.countTransmit ++;
			uartStruct.sizeToTransmit--;
			//change to receive
			uartStruct.uartFreeFlag = UART_IS_FREE;
			uartSwitchToReceive();
		}
	}

}
/*
 *	Transmit data
 *		dataToSend - pointer to data which has to be send
 *		sizeOfData - amount of data to send
 */
static void uartTransmit(uint8_t *dataToSend, uint32_t sizeOfData)
{
	while(uartStruct.uartFreeFlag == UART_IS_BUSY)		//wait until send data
	{
		timerDelayUs(20);
	}
	uartStruct.uartFreeFlag = UART_IS_BUSY;

	//copy data
	for(uint32_t i=0; i<sizeOfData; i++)
	{
		uartStruct.uartTransmitBuffer[i] = dataToSend[i];
	}
	uartStruct.sizeToTransmit = sizeOfData;
	uartStruct.countTransmit = 0;
	uartSwitchToTransmit();
}

/*
 *
 * To print some message
 */
void uartPrintf(const char *mesToPrint)
{
	uint16_t textToPrintSize = 1024;
	for(int i=0; i<255; i++)
	{
		if(mesToPrint[i] == '\0')
		{
			textToPrintSize = i;
			i = 255;
		}
	}

	// jeżeli był bląd
	if(textToPrintSize == 1024)
	{
		uint8_t errorMesg[] = "Brakuje znaku \\0 !\n";
		uartTransmit(errorMesg,sizeof(errorMesg));
	}
	else
	{
		uartTransmit((uint8_t *)mesToPrint, (uint32_t)textToPrintSize);
	}
}


static void uartSwitchToReceive(void)
{
	UART_PERH->CR1 &= ~USART_CR1_TXEIE;
	UART_PERH->RDR;		//read to clear register
	UART_PERH->CR1 |= USART_CR1_RXNEIE;
}

static void uartSwitchToTransmit(void)
{
	while( UART_PERH->ISR & USART_ISR_RXNE);
	UART_PERH->CR1 &= ~USART_CR1_RXNEIE;
	UART_PERH->CR1 |= USART_CR1_TXEIE;
}
