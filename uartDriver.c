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
static void gpioInit(void);
static void stopReceiving(void);
static void stopTransmiting(void);
static void startReceiving(void);
static void startTransmitting(void);
static void transmitData(uint8_t *, uint32_t);


typedef struct{
	uint32_t countReceived;
	uint8_t uartReveiveBuffer[UART_BUFFER_SIZE];
	uint32_t countTransmit;
	uint32_t sizeToTransmit;
	uint8_t uartTransmitBuffer[UART_BUFFER_SIZE];
	uint8_t uartFreeFlag;		// it show if uart is free to send data
}UarStruct;
UarStruct uartStruct;

/*
 *  Uart perh init
 */
void uartInit(void)
{
	gpioInit();
	perhUartClockEnable();

	//disable module
	UART_PERH->CR1 &= ~USART_CR1_UE;

	//set baudrate
	UART_PERH->BRR = USARTDIV_16_OVR & 0xffff;

	//enable module
	UART_PERH->CR1 |= USART_CR1_UE;

	//CR reg setup
	UART_PERH->CR1 |=	USART_CR1_RE		// Receiver mode enable
						|USART_CR1_TE;		//Transmitter mode enable

	//set interrupt Priority
	NVIC_SetPriority(UART_PERH_IRQ, 1);
	NVIC_EnableIRQ(UART_PERH_IRQ);

	uartStruct.uartFreeFlag = UART_IS_FREE;		//uart is free
	stopReceiving();
	stopTransmiting();
}

/*
 * Initialize uart port GPIO
 */
static inline void gpioInit(void)
{
	portUartClockEnable();

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

	//set pull up
	UART_TX_PORT->PUPDR |= GPIO_PUPDR_PUPD0_0 << (UART_TX_PIN * 2);
	UART_RX_PORT->PUPDR |= GPIO_PUPDR_PUPD0_0 << (UART_RX_PIN * 2);

	//alternative active
	UART_TX_PORT->AFR[0] |= GPIO_AF7_USART3 << (UART_TX_PIN * 4);
	UART_RX_PORT->AFR[0] |= GPIO_AF7_USART3 << (UART_RX_PIN * 4);
}

void UART_PERH_IRQHandler(void)
{
	if((UART_PERH->ISR & USART_ISR_RXNE) && (UART_PERH->CR1 & USART_CR1_RXNEIE))
	{
		uartStruct.uartReveiveBuffer[uartStruct.countReceived ++] = UART_PERH->RDR;
	}
	if((UART_PERH->ISR & USART_ISR_TXE) && (UART_PERH->CR1 & USART_CR1_TXEIE))
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
			uartStruct.uartFreeFlag = UART_IS_FREE;
			//change to receive
			stopTransmiting();
		}
	}
	if(UART_PERH->ISR & USART_ISR_ORE)
	{
		UART_PERH->ICR |= USART_ICR_ORECF;	//Clear flag
	}
}

/*
 *	Transmit data
 *		dataToSend - pointer to data which has to be send
 *		sizeOfData - amount of data to send
 */
static void transmitData(uint8_t *dataToSend, uint32_t sizeOfData)
{
	while(uartStruct.uartFreeFlag == UART_IS_BUSY)		//wait until send data
	{
		timerDelayUs(300);
	}
	uartStruct.uartFreeFlag = UART_IS_BUSY;

	//copy data
	for(uint32_t i=0; i<sizeOfData; i++)
	{
		uartStruct.uartTransmitBuffer[i] = dataToSend[i];
	}

	uartStruct.sizeToTransmit = sizeOfData;
	uartStruct.countTransmit = 0;
	startTransmitting();
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
		uint8_t errorMesg[] = "Brakuje znaku \\0!\n";
		transmitData(errorMesg,sizeof(errorMesg));
	}
	else
	{
		transmitData((uint8_t *)mesToPrint, (uint32_t)textToPrintSize);
	}
}


static void startReceiving(void)
{
	while(!(UART_PERH->ISR & USART_ISR_TC));
	UART_PERH->CR1 |= USART_CR1_RXNEIE;
}

static void startTransmitting(void)
{
	while(!(UART_PERH->ISR & USART_ISR_TC));
	UART_PERH->CR1 |= USART_CR1_TXEIE;
}

/*
 *  Module stop receiving data
 */
static void stopReceiving(void)
{
	UART_PERH->CR1 &= ~USART_CR1_RXNEIE;
}

/*
 *  Module stop transmiting data
 */
static void stopTransmiting(void)
{
	UART_PERH->CR1 &= ~USART_CR1_TXEIE;
}

/*
 * Prepared empy buffer for data
 */
void uartClearBuffer(void)
{
	uartStruct.countReceived = 0;
}

void uartStartReceivingData(void)
{
	startReceiving();
}
void uartStopReceivingData(void)
{
	stopReceiving();
}

void uartGetReceivedData(uint8_t *dataBuffer)
{
	for(uint32_t i=0; i< uartStruct.countReceived; i++)
	{
		dataBuffer[i] = uartStruct.uartReveiveBuffer[i];
	}
}
