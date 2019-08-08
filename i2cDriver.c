/*
 * i2cDriver.c
 *
 *  Created on: Jun 3, 2019
 *      Author: lukasz
 */
#include "main.h"
#include "i2cDriver.h"

//private function
static void sendStopBit(void);

/*
 * 	Init I2C GPIO
 * 	PB6 -> SCL
 * 	PB7 -> SDA
 */
inline void i2cGpioInit(void)
{
	//clock enable
	portClockEnable();

	//set up MODER REG
	I2C_SDA_PORT->MODER &= ~(GPIO_MODER_MODE0_Msk << (I2C_SDA_PIN  * 2));	//clear mode flag
	I2C_SDA_PORT->MODER |= GPIO_MODER_MODE0_1 << (I2C_SDA_PIN * 2);		//alternative function

	I2C_SCL_PORT->MODER &= ~(GPIO_MODER_MODE0_Msk << (I2C_SCL_PIN  * 2));	//clear mode flag
	I2C_SCL_PORT->MODER |= GPIO_MODER_MODE0_1 << (I2C_SCL_PIN  * 2);		//alternative function

	//Setup OSPEEDR reg, speed work for pin
	I2C_SDA_PORT->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0_Msk << (I2C_SDA_PIN  * 2));
	I2C_SDA_PORT->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0_1 << (I2C_SDA_PIN  * 2);	// Set speed to high

	I2C_SCL_PORT->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0_Msk << (I2C_SCL_PIN  * 2));
	I2C_SCL_PORT->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0_1 << (I2C_SCL_PIN  * 2);

	//set open drain
	I2C_SDA_PORT->OTYPER |= GPIO_OTYPER_OT0 << I2C_SDA_PIN;
	I2C_SCL_PORT->OTYPER |= GPIO_OTYPER_OT0 << I2C_SCL_PIN;
	//pull uo
	I2C_SCL_PORT->PUPDR |= GPIO_PUPDR_PUPD0_0 << I2C_SCL_PIN;
	I2C_SDA_PORT->PUPDR |= GPIO_PUPDR_PUPD0_0 << I2C_SDA_PIN;

	//alternate setup
#if(I2C_SCL_PIN < 8)
	I2C_SCL_PORT->AFR[0] |= GPIO_AF4_I2C1 << (I2C_SCL_PIN * 4);
#else
	I2C_SCL_PORT->AFR[1] |= GPIO_AF4_I2C1 << ( (I2C_SCL_PIN - 8) * 4);
#endif

#if(I2C_SDA_PIN < 8)
	I2C_SDA_PORT->AFR[0] |= GPIO_AF4_I2C1 << (I2C_SDA_PIN * 4);
#else
	I2C_SDA_PORT->AFR[1] |= GPIO_AF4_I2C1 << ( (I2C_SDA_PIN - 8) * 4);
#endif
}

/*
 *  I2C interface init
 */
inline void i2cInit(void)
{
	i2cClockEnable();

	//timing configuration for 100 kHz.
	I2C_PERH->TIMINGR = (0x7 << I2C_TIMINGR_PRESC_Pos)
						| (0x13 <<I2C_TIMINGR_SCLL_Pos)
						| (0xF <<I2C_TIMINGR_SCLH_Pos)
						| (0x2 <<I2C_TIMINGR_SDADEL_Pos)
						| (0x4 <<I2C_TIMINGR_SCLDEL_Pos);

	I2C_PERH->CR1 |= I2C_CR1_PE;	// perph enable
}

/*
 *  Function to ask I2C for reg
 *  arg:
 *  	deviceAddr - device address
 *  	regAddr	- register address
 */
uint8_t i2CReadReg(uint8_t deviceAddr,uint8_t regAddr)
{
	char receivedValue = 0;

	//Init configuration =======================================
	I2C_PERH->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_SADD);	//clear fields
	I2C_PERH->CR2 |= 1 << I2C_CR2_NBYTES_Pos;	//bytes to transmit
	I2C_PERH->CR2 |= deviceAddr;
	I2C_PERH->CR2 &= ~I2C_CR2_RD_WRN;			// Transfer direction - write byte
	I2C_PERH->CR2 |= I2C_CR2_START;				//start communication

	while(!((I2C1->ISR & I2C_ISR_TXE ) | (I2C1->ISR & I2C_ISR_NACKF )));	//Wait until address sended
	if(I2C1->ISR & I2C_ISR_NACKF)
	{
		return 0;
	}

	I2C_PERH->TXDR = regAddr;					//send register
	while(!(I2C_PERH->ISR & I2C_ISR_TC));

	I2C_PERH->CR2 |= I2C_CR2_RD_WRN;		//change to read mode
	I2C_PERH->CR2 |= I2C_CR2_START;

	//Fetch data
	while(!(I2C_PERH->ISR & I2C_ISR_RXNE));
	receivedValue = I2C_PERH->RXDR;

	sendStopBit();
	return receivedValue;
}

/*
 *  Function to write using I2C in reg
 *  arg:
 *  	deviceAddr - device address
 *  	regAddr	- register address
 *  	dataToWrite
 */
void i2CWriteReg(uint8_t deviceAddr,uint8_t regAddr,uint8_t dataToWrite)
{
	//Init configuration =======================================
	I2C_PERH->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_SADD);	//clear fields
	I2C_PERH->CR2 |= 2 << I2C_CR2_NBYTES_Pos;	//bytes to transmit
	I2C_PERH->CR2 |= deviceAddr;
	I2C_PERH->CR2 &= ~I2C_CR2_RD_WRN;			//write byte
	I2C_PERH->CR2 |= I2C_CR2_START;				//start communicatioin

	while(!((I2C1->ISR & I2C_ISR_TXE ) | (I2C1->ISR & I2C_ISR_NACKF )));	//Wait until address sended
	if(I2C1->ISR & I2C_ISR_NACKF)
	{
		return;
	}

	I2C_PERH->TXDR = regAddr;					//send register
	while(!(I2C_PERH->ISR & I2C_ISR_TXE));		// Wait until buffer empty
	I2C_PERH->TXDR = dataToWrite;

	while(!(I2C_PERH->ISR & I2C_ISR_TC));		//wait until transmit end

	sendStopBit();
}



/*
 *  Function to ask I2C for mulpiple bytes
 *  arg:
 *  	deviceAddr - device adress
 *  	regStartAddr - register where start reading
 *  	dataBuffer - buffer where data will be stored
 *  	dataSize - how many bytes to read
 */
void i2CReadMultipleBytes(	uint8_t deviceAddr,
								uint8_t regStartAddr,
								uint8_t * dataBuffer,
								uint16_t dataSize	)
{
	//Init configuration =======================================
	I2C_PERH->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_SADD);	//clear fields
	I2C_PERH->CR2 |= 1 << I2C_CR2_NBYTES_Pos;	//bytes to transmit
	I2C_PERH->CR2 |= deviceAddr;
	I2C_PERH->CR2 &= ~I2C_CR2_RD_WRN;			//write byte
	I2C_PERH->CR2 |= I2C_CR2_START;				//start communicatioin

	while(!((I2C1->ISR & I2C_ISR_TXE ) | (I2C1->ISR & I2C_ISR_NACKF )));	//Wait until adress sended
	if(I2C1->ISR & I2C_ISR_NACKF)
	{
		return;
	}

	I2C_PERH->TXDR = regStartAddr;					//send register
	while(!(I2C_PERH->ISR & I2C_ISR_TC));

	I2C_PERH->CR2 |= I2C_CR2_RD_WRN;		//change to read mode
	I2C_PERH->CR2 |= dataSize << I2C_CR2_NBYTES_Pos;	//bytes to transmit
	I2C_PERH->CR2 |= I2C_CR2_START;

	for(uint32_t i = 0; i< dataSize;  i++)
	{
		if(i == dataSize - 1)
		{
			I2C_PERH->CR2 |= I2C_CR2_STOP;
		}
		//Fetch data
		while(!(I2C_PERH->ISR & I2C_ISR_RXNE));
		dataBuffer[i] = I2C_PERH->RXDR;
	}


	sendStopBit();
}
/*
 * 	Ends comunication by sending stop bit
 */
static void sendStopBit(void)
{
	I2C_PERH->CR2 |= I2C_CR2_STOP;
	while(!(I2C_PERH->ISR & I2C_ISR_STOPF));	//wait until stop send
	I2C_PERH->ICR |= I2C_ICR_STOPCF;			//clear stop flag
}
