/*
 * Autor: Nico Korn
 * Date: 26.10.2017
 * Firmware for the STM32F4Discovery Board to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * uart_bt.c this module contents the initialization of the gpio's and uart
 * communication with the bluetooth module.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include "gpio_ws2812.h"
#include "stm32f4xx.h"


//defines
#define BT_BAUD 9600
#define RXBUFFERSIZE 867
#define LEDBUFFERSIZE 865


//variables
GPIO_InitTypeDef GPIO_InitStruct; 	// this is for the GPIO pins used as TX and RX
UART_HandleTypeDef USART_TypeDefStruct;
uint8_t aRxBuffer[RXBUFFERSIZE];		// Buffer used for reception
uint8_t aLEDBuffer[LEDBUFFERSIZE];		// Buffer used for reception
uint8_t mode = 0x01;
uint8_t function = 0x00;

//function
void usart_init_bt(void){

	/* Enable peripherals and GPIO Clocks */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE();

	/* Set TX RX pin */
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6; 				// Pin 5 (TX) | Pin 6 (RX)
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; 					// the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;				// this defines the IO speed and has nothing to do with the baudrate!
	GPIO_InitStruct.Pull = GPIO_PULLUP;							// this activates the pullup resistors on the IO pins... old: GPIO_PULLUP;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);						// setting GPIO registers

	/* USART */
	USART_TypeDefStruct.Init.BaudRate = BT_BAUD;				// baudrate is set to 9600
	USART_TypeDefStruct.Init.WordLength = UART_WORDLENGTH_8B;	// we want the data frame size to be 8 bits (standard)
	USART_TypeDefStruct.Init.StopBits = UART_STOPBITS_1;		// we want 1 stop bit (standard)
	USART_TypeDefStruct.Init.Parity = UART_PARITY_NONE;			// we don't want a parity bit (standard)
	USART_TypeDefStruct.Init.Mode = UART_MODE_TX_RX; 			// we want to enable the transmitter and the receiver
	USART_TypeDefStruct.Init.OverSampling = UART_OVERSAMPLING_16;
	USART_TypeDefStruct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	USART_TypeDefStruct.Instance = USART2;
	HAL_UART_Init(&USART_TypeDefStruct);						// again all the properties are passed to the USART_Init function which takes care of all the bit setting

	/* NVIC configuration for USART2 receive interrupt */
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
	/* Enable interrupt */
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	/* Put USART peripheral in reception process */
	if(HAL_UART_Receive_IT(&USART_TypeDefStruct, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK){
		while(1){
			//error, wait here
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART2){
	    // Your buffer is full with your data process it here or set a flag
		/* read rx buffer */
		uint16_t i;
		mode = aRxBuffer[0];
		function = aRxBuffer[1];
		for(i=0; i<LEDBUFFERSIZE; i++){
			aLEDBuffer[i] = aRxBuffer[i+2];
		}

		/* Put USART peripheral in interrupt driven reception process */
		if(HAL_UART_Receive_IT(&USART_TypeDefStruct, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK){
			while(1){
				//error, wait here
			}
		}

	}
}

void get_rx_data_buffer(uint8_t *prx_buffer){
	prx_buffer = *aRxBuffer;
}
