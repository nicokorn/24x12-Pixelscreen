/*
 * Autor: Nico Korn
 * Date: 26.10.2017
 * Firmware for the STM32F4Discovery Board to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * Main.c this module contents the main routine. All peripherals are initialized
 * here plus setting the led colors and sending the information is done here.
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

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f4xx.h"


// ----------------------------------------------------------------------------
//
// Standalone STM32F4 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//defines
#define RXBUFFERSIZE 867
#define LEDBUFFERSIZE 865
#define COLS 24
#define ROWS 12

//variables
extern uint8_t mode;
extern uint8_t function;
extern uint8_t aLEDBuffer[LEDBUFFERSIZE];			// Buffer used for reception
uint8_t fx_index = 0;

//main routine
int main(void){
	HAL_Init();
	SystemClock_Config();
	gpio_init_ws2812();
	dma_init_ws2812();
	timer_init_ws2812();
	usart_init_bt();
	rtc_init_2812();
	fx_init();
	accelerometer_io_init();
	accelerometer_init();

	while (1){
		/*copy data from rx buffer*/
		//read_rx_buffer();
		/*led device fsm*/
		switch(mode){
			/* FX mode */
			case 0x00: 	switch(function){
							case 0x00: 	fx_index++;
										function = 0xff;
							break;
							case 0x01: 	fx_index--;
										function = 0xff;
							break;
							case 0x10: 	picture_load(aLEDBuffer, LEDBUFFERSIZE, COLS, ROWS);
							break;
							case 0x11: 	led_test();
							break;
							default:	fx_chooser(fx_index);
						};
			break;
			/* clock mode */
			case 0x01: 	switch(function){
							case 0x00: 	function = 0xff;
							break;
							case 0x01: 	function = 0xff;
							break;
							case 0x10: 	led_clock_hour_plus();
										function = 0xff;
							break;
							case 0x11: 	led_clock_hour_minus();
										function = 0xff;
							break;
							case 0x20: 	led_clock_minute_plus();
										function = 0xff;
							break;
							case 0x21: 	led_clock_minute_minus();
										function = 0xff;
							break;
							default:	show_time();
						};
			break;
			/* clock wasserwaage */
							case 0x02:	accelerometer_mode();
			break;
			default: 	clear_ws2812_leds();
						// send the framebuffer out to the LEDs
						sendbuf_WS2812();
						// 25 ms refresh rate
						HAL_Delay(100);
		}
		/*reset function but keep mode*/
		//function = 0xff;
	}
	return 0;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}
/*
void read_rx_buffer(void){
	uint16_t i;
	get_rx_data_buffer((uint8_t *)led_rx_buffer);
	mode = led_rx_buffer[0];
	function = led_rx_buffer[1];
	for(i=0; i<LEDBUFFERSIZE; i++){
		aLEDBuffer[i] = led_rx_buffer[i+2];
	}
}
*/
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
