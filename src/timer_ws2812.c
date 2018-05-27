/*
 * Autor: Nico Korn
 * Date: 26.10.2017
 * Firmware for the STM32F4Discovery Board to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * timer_ws2812.c this module contents the initialization of the timer.
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

#include "timer_ws2812.h"
#include "stm32f4xx.h"

//defines

//variables
TIM_HandleTypeDef TIM_HandleStruct;

//function
void timer_init_ws2812(void){
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef TIM_OC1Struct;
	TIM_OC_InitTypeDef TIM_OC2Struct;
	uint16_t PrescalerValue;

	// TIM1 Periph clock enable
	__HAL_RCC_TIM1_CLK_ENABLE();

	// set prescaler to get a 24 MHz clock signal
	PrescalerValue = (uint16_t) (168000000 / 24000000) - 1;

	/* Time base configuration */
	TIM_HandleStruct.Instance = TIM1;
	TIM_HandleStruct.Init.Period = 29;	// set the period to get 29 to get a 800kHz timer
	TIM_HandleStruct.Init.Prescaler = PrescalerValue;
	TIM_HandleStruct.Init.ClockDivision = 0;
	TIM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TIM_HandleStruct);
    // Reset the ARR Preload Bit
	TIM1->CR1 &= (uint16_t)~TIM_CR1_ARPE;


	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&TIM_HandleStruct, &sClockSourceConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&TIM_HandleStruct, &sMasterConfig);

	/* Timing Mode configuration: Channel 1 */
	TIM_OC1Struct.OCMode = TIM_OCMODE_TIMING;
	TIM_OC1Struct.Pulse = 8;
	// Configure the channel
	HAL_TIM_OC_ConfigChannel(&TIM_HandleStruct, &TIM_OC1Struct, TIM_CHANNEL_1);

	/* Timing Mode configuration: Channel 2 */
	TIM_OC2Struct.OCMode = TIM_OCMODE_PWM1;
	TIM_OC2Struct.Pulse = 17;
	TIM_OC2Struct.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_OC2Struct.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	TIM_OC2Struct.OCIdleState = TIM_OCIDLESTATE_SET;
	TIM_OC2Struct.OCFastMode = TIM_OCFAST_ENABLE;
	TIM_OC2Struct.OCIdleState = TIM_CCx_ENABLE;
	// Configure the channel
	HAL_TIM_PWM_ConfigChannel(&TIM_HandleStruct, &TIM_OC2Struct, TIM_CHANNEL_2);

	/* configure TIM1 interrupt */
	HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1, 2);
	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}
