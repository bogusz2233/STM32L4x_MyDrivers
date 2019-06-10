/*
 * uartDriver.h
 *
 *  Created on: Jun 4, 2019
 *      Author: lukasz
 */

#ifndef STM32L4X_MYDRIVER_UARTDRIVER_H_
#define STM32L4X_MYDRIVER_UARTDRIVER_H_

//Port clock enable
#define portUartClockEnable()		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN

#define perhUartClockEnable()		RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN

// Pins
#define UART_TX_PIN		2
#define UART_TX_PORT		GPIOA

#define UART_RX_PIN		3
#define UART_RX_PORT		GPIOA

#define UART_PERH			USART2
#define UART_PERH_IRQ		USART2_IRQn
#define  UART_PERH_IRQHandler	USART2_IRQHandler

#define UART_BAUDRATE		115200
#define USARTDIV_16_OVR	( (uint32_t) CLOCK_FREQ_MHZ *1000000 / UART_BAUDRATE )

#define UART_BUFFER_SIZE	500

//State descrive
#define UART_IS_BUSY		0
#define UART_IS_FREE		1
typedef struct{
	uint32_t countReceived;
	uint8_t uartReveiveBuffer[UART_BUFFER_SIZE];
	uint32_t countTransmit;
	uint32_t sizeToTransmit;
	uint8_t uartTransmitBuffer[UART_BUFFER_SIZE];
	uint8_t uartFreeFlag;		// it show if uart is free to send data
}UartStruct;

//public function
void uartInit(void);
void uartPrintf(const char *);

#endif /* STM32L4X_MYDRIVER_UARTDRIVER_H_ */