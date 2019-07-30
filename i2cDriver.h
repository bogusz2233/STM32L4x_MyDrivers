/*
 * i2cDriver.h
 *
 *  Created on: Jun 3, 2019
 *      Author: lukasz
 */

#ifndef STM32L4X_MYDRIVER_I2CDRIVER_H_
#define STM32L4X_MYDRIVER_I2CDRIVER_H_

//Port clock enable
#define portClockEnable()		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN

//Periph clock enable
#define i2cClockEnable()		RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN

//SCL pin
#define I2C_SCL_PORT	GPIOA
#define I2C_SCL_PIN	9

//SDA pin
#define I2C_SDA_PORT	GPIOA
#define I2C_SDA_PIN	10

#define I2C_PERH		I2C1

//public function:
void i2cInit(void);
void i2cGpioInit(void);
uint8_t i2CReadReg(uint8_t,uint8_t);
void i2CReadMultipleBytes(uint8_t,uint8_t,uint8_t *, uint16_t);
void i2CWriteReg(uint8_t,uint8_t,uint8_t);

#endif /* STM32L4X_MYDRIVER_I2CDRIVER_H_ */
