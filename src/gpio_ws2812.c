/*
 * Autor: Nico Korn
 * Date: 26.10.2017
 * Firmware for the STM32F4Discovery Board to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * gpio_ws2812.c this module contents the initialization of the gpio's.
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

//variables

//function
void gpio_init_ws2812(void){
	__HAL_RCC_GPIOC_CLK_ENABLE();						//enable clock on the bus
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = (uint16_t)0xFFFFU; 			// select all 16 Pins... To Do: Error on PINS 14 & 15
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 		// configure pins for pp output
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	// 50 MHz rate
	GPIO_InitStruct.Pull = GPIO_NOPULL;					// this activates the pullup resistors on the IO pins
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);				// setting GPIO registers
}


