/*
 * Autor: Nico Korn
 * Date: 26.10.2017
 * Firmware for the STM32F4Discovery Board to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * fx_ws2812.c this module contents led effects algorithms.
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

#include "clock_ws2812.h"
#include "stm32f4xx.h"

//defines

//variables
extern uint8_t 						WS2812_TC;	// global variable: TC=1 transfer completed, TC=0 transfer not completed
/* Buffers used for displaying Time and Date */
RTC_HandleTypeDef 					RTC_Handle;
RTC_TimeTypeDef 					timestructure;
RTC_DateTypeDef 					datestructure;
RTC_TamperTypeDef 					tamperstructure;
uint8_t 							double_points = 0;
uint16_t 							x_point = 0;
uint16_t 							y_point = 0;
uint16_t 							counter = 0;

/*
 * initialization of rtc timer
 */
//function
void rtc_init_2812(){

	RCC_OscInitTypeDef        RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

	/* Configue LSE as RTC clock soucre */
	RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;

	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
		//error, stay here
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
	  //error, stay here
	}

	/* Enable RTC Clock */
	__HAL_RCC_RTC_ENABLE();

	RTC_Handle.Instance = 			RTC;
	RTC_Handle.Init.HourFormat = 	RTC_HOURFORMAT_24;
	RTC_Handle.Init.AsynchPrediv = 	127;
	RTC_Handle.Init.SynchPrediv = 	255;
	RTC_Handle.Init.OutPut = 		RTC_OUTPUT_DISABLE;
	RTC_Handle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	RTC_Handle.Init.OutPutType = 	RTC_OUTPUT_TYPE_OPENDRAIN;

	if(HAL_RTC_Init(&RTC_Handle) != HAL_OK){
		//Error, stay here
	}

	/* Check if Data stored in BackUp register0: No Need to reconfigure RTC */
	/* Read the BackUp Register 0 Data */
	if(HAL_RTCEx_BKUPRead(&RTC_Handle, RTC_BKP_DR0) != 0x32F2){
		/* Configure RTC Calendar */
		/* Set Date: Tuesday February 18th 2014 */
		datestructure.Year = 0x14;
		datestructure.Month = RTC_MONTH_FEBRUARY;
		datestructure.Date = 0x18;
		datestructure.WeekDay = RTC_WEEKDAY_TUESDAY;

		if(HAL_RTC_SetDate(&RTC_Handle,&datestructure,FORMAT_BCD) != HAL_OK){
			//Error, stay here
		}

		/* Set Time: 08:10:00 */
		timestructure.Hours = 0x08;
		timestructure.Minutes = 0x10;
		timestructure.Seconds = 0x00;
		timestructure.TimeFormat = RTC_HOURFORMAT_24;
		timestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
		timestructure.StoreOperation = RTC_STOREOPERATION_RESET;

		if(HAL_RTC_SetTime(&RTC_Handle,&timestructure,FORMAT_BCD) != HAL_OK){
			//Error, stay here
		}

		/* Writes a data in a RTC Backup data Register0 */
		HAL_RTCEx_BKUPWrite(&RTC_Handle,RTC_BKP_DR0,0x32F2);
	}else{
	    /* Check if the Power On Reset flag is set */
	    if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET){
	      //Power on reset occured, stay here
	    }

	    /* Check if Pin Reset flag is set */
	    if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET){
	    	//stay here
	    }

	    /* Clear Reset Flag */
	    __HAL_RCC_CLEAR_RESET_FLAGS();
	  }
}

void show_time(){
	RTC_TimeTypeDef timestructureget;
	RTC_DateTypeDef datestructureget;
	HAL_RTC_GetTime(&RTC_Handle, &timestructureget, FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_Handle, &datestructureget, FORMAT_BIN);
	/* coordinates for the numbers */
	uint16_t x1 = 1;
	uint16_t y1 = 3;
	uint16_t x2 = 6;
	uint16_t y2 = 3;
	uint16_t x3 = 13;
	uint16_t y3 = 3;
	uint16_t x4 = 18;
	uint16_t y4 = 3;
	/* hours and minutes local places: hh:mm = h0h1:m0m2 */
	uint16_t h0 = 0;
	uint16_t h1 = 0;
	uint16_t m0 = 0;
	uint16_t m1 = 0;
	uint16_t s01 = 0;

	/* setup display */
	clear_ws2812_leds();

	/* set background */
	set_ws2812_leds_background();

	/* setup segment structure */
	led_clock_segment(x1,y1);
	led_clock_segment(x2,y2);
	led_clock_segment(x3,y3);
	led_clock_segment(x4,y4);

	/* process hour */
	h1 = timestructureget.Hours % 10;
	h0 = (timestructureget.Hours - h1) / 10;

	/* process minutes */
	m1 = timestructureget.Minutes % 10;
	m0 = (timestructureget.Minutes - m1) / 10;

	/* process seconds */
	s01 = timestructureget.Seconds;

	/* prepare numbers for display */
	switch(h0){
		case 0:	led_clock_segment_zero(x1,y1);
		break;
		case 1:	led_clock_segment_one(x1,y1);
		break;
		case 2:	led_clock_segment_two(x1,y1);
		break;
	}

	switch(h1){
		case 0:	led_clock_segment_zero(x2,y2);
		break;
		case 1:	led_clock_segment_one(x2,y2);
		break;
		case 2:	led_clock_segment_two(x2,y2);
		break;
		case 3:	led_clock_segment_three(x2,y2);
		break;
		case 4:	led_clock_segment_four(x2,y2);
		break;
		case 5:	led_clock_segment_five(x2,y2);
		break;
		case 6:	led_clock_segment_six(x2,y2);
		break;
		case 7:	led_clock_segment_seven(x2,y2);
		break;
		case 8:	led_clock_segment_eight(x2,y2);
		break;
		case 9:	led_clock_segment_nine(x2,y2);
		break;
	}

	switch(m0){
		case 0:	led_clock_segment_zero(x3,y3);
		break;
		case 1:	led_clock_segment_one(x3,y3);
		break;
		case 2:	led_clock_segment_two(x3,y3);
		break;
		case 3:	led_clock_segment_three(x3,y3);
		break;
		case 4:	led_clock_segment_four(x3,y3);
		break;
		case 5:	led_clock_segment_five(x3,y3);
		break;
		case 6:	led_clock_segment_six(x3,y3);
		break;
	}

	switch(m1){
		case 0:	led_clock_segment_zero(x4,y4);
		break;
		case 1:	led_clock_segment_one(x4,y4);
		break;
		case 2:	led_clock_segment_two(x4,y4);
		break;
		case 3:	led_clock_segment_three(x4,y4);
		break;
		case 4:	led_clock_segment_four(x4,y4);
		break;
		case 5:	led_clock_segment_five(x4,y4);
		break;
		case 6:	led_clock_segment_six(x4,y4);
		break;
		case 7:	led_clock_segment_seven(x4,y4);
		break;
		case 8:	led_clock_segment_eight(x4,y4);
		break;
		case 9:	led_clock_segment_nine(x4,y4);
		break;
	}

	/* led the double point blink */
	if(counter < 100){
		led_clock_segment_middle(11,3);
	}else if(counter > 100){

	}
	if(counter > 200){
		counter = 0;
	}
	counter++;

	/* running point */
	WS2812_framedata_setPixel(y_point, x_point, 0x00, 0x00, 0xFF);
	if(x_point < 23 && y_point == 0){
		x_point++;
	}else if(x_point == 23 && y_point < 11){
		y_point++;
	}else if(x_point > 0 && y_point == 11){
		x_point--;
	}else if(x_point == 0 && y_point > 0){
		y_point--;
	}

	/* send the framebuffer out to the LEDs */
	sendbuf_WS2812();
	/* 10 ms refresh rate */
	HAL_Delay(10);
}

/*
 * nacked seven segment
 */
void led_clock_segment(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0x01);

	//left side
	WS2812_framedata_setPixel(y+1, x+0, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+2, x+0, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+4, x+0, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+5, x+0, 0x00, 0x00, 0x01);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0x01);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0x01);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0x01);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0x01);
}

/*
 * double points in the middle
 */
void led_clock_segment_middle(uint16_t x, uint8_t y){
	//double points
	WS2812_framedata_setPixel(y+2, x, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x, 0x00, 0x00, 0xFF);
}

/*
 * number nine
 */
void led_clock_segment_nine(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//left side
	WS2812_framedata_setPixel(y+1, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+0, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0xFF);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number eight
 */
void led_clock_segment_eight(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//left side
	WS2812_framedata_setPixel(y+1, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+0, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0xFF);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number seven
 */
void led_clock_segment_seven(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number six
 */
void led_clock_segment_six(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//left side
	WS2812_framedata_setPixel(y+1, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+0, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0xFF);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number five
 */
void led_clock_segment_five(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//left side
	WS2812_framedata_setPixel(y+1, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+0, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0xFF);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number four
 */
void led_clock_segment_four(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//left side
	WS2812_framedata_setPixel(y+1, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+0, 0x00, 0x00, 0xFF);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number three
 */
void led_clock_segment_three(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0xFF);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number two
 */
void led_clock_segment_two(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);

	//left side
	WS2812_framedata_setPixel(y+4, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+0, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);

	//middle
	WS2812_framedata_setPixel(y+3, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+3, x+2, 0x00, 0x00, 0xFF);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0xFF);
}

/*
 * number one
 */
void led_clock_segment_one(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);
}

/*
 * number zero
 */
void led_clock_segment_zero(uint16_t x, uint8_t y){
	//right side
	WS2812_framedata_setPixel(y+1, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+3, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+3, 0x00, 0x00, 0xFF);

	//left side
	WS2812_framedata_setPixel(y+1, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+2, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+4, x+0, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+5, x+0, 0x00, 0x00, 0xFF);

	//up
	WS2812_framedata_setPixel(y+0, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+0, x+2, 0x00, 0x00, 0xFF);

	//down
	WS2812_framedata_setPixel(y+6, x+1, 0x00, 0x00, 0xFF);
	WS2812_framedata_setPixel(y+6, x+2, 0x00, 0x00, 0xFF);

}

/*
 * set hour +1
 */
void led_clock_hour_plus(){
	RTC_TimeTypeDef timestructureset;
	RTC_DateTypeDef datestructureset;
	RTC_TimeTypeDef timestructureget;
	RTC_DateTypeDef datestructureget;
	HAL_RTC_GetTime(&RTC_Handle, &timestructureget, FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_Handle, &datestructureget, FORMAT_BIN);

	if(timestructureget.Hours + 0x01 < 0x18){
		timestructureset.Hours = timestructureget.Hours + 0x01;
	}else{
		timestructureset.Hours = 0x00;
	}
	timestructureset.Minutes = timestructureget.Minutes;
	timestructureset.Seconds = timestructureget.Seconds;
	timestructureset.TimeFormat = RTC_HOURFORMAT_24;
	timestructureset.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	timestructureset.StoreOperation = RTC_STOREOPERATION_RESET;

	datestructureset.Year = datestructureget.Year;
	datestructureset.Month = datestructureget.Month;
	datestructureset.Date = datestructureget.Date;
	datestructureset.WeekDay = datestructureget.WeekDay;

	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, FORMAT_BIN);
	HAL_RTC_SetDate(&RTC_Handle, &datestructureset, FORMAT_BIN);

}

/*
 * set hour -1
 */
void led_clock_hour_minus(){
	RTC_TimeTypeDef timestructureset;
	RTC_DateTypeDef datestructureset;
	RTC_TimeTypeDef timestructureget;
	RTC_DateTypeDef datestructureget;
	HAL_RTC_GetTime(&RTC_Handle, &timestructureget, FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_Handle, &datestructureget, FORMAT_BIN);

	if(timestructureget.Hours - 0x01 > 0x00){
		timestructureset.Hours = timestructureget.Hours - 0x01;
	}else{
		timestructureset.Hours = 0x17;
	}
	timestructureset.Minutes = timestructureget.Minutes;
	timestructureset.Seconds = timestructureget.Seconds;
	timestructureset.TimeFormat = RTC_HOURFORMAT_24;
	timestructureset.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	timestructureset.StoreOperation = RTC_STOREOPERATION_RESET;

	datestructureset.Year = datestructureget.Year;
	datestructureset.Month = datestructureget.Month;
	datestructureset.Date = datestructureget.Date;
	datestructureset.WeekDay = datestructureget.WeekDay;

	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, FORMAT_BIN);
	HAL_RTC_SetDate(&RTC_Handle, &datestructureset, FORMAT_BIN);
}

/*
 * set minutes +1
 */
void led_clock_minute_plus(){
	RTC_TimeTypeDef timestructureset;
	RTC_DateTypeDef datestructureset;
	RTC_TimeTypeDef timestructureget;
	RTC_DateTypeDef datestructureget;
	HAL_RTC_GetTime(&RTC_Handle, &timestructureget, FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_Handle, &datestructureget, FORMAT_BIN);

	timestructureset.Hours = timestructureget.Hours;
	if(timestructureget.Minutes + 0x01 < 0x3c){
		timestructureset.Minutes = timestructureget.Minutes + 0x01;
	}else{
		timestructureset.Minutes = 0x00;
	}
	timestructureset.Seconds = timestructureget.Seconds;
	timestructureset.TimeFormat = RTC_HOURFORMAT_24;
	timestructureset.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	timestructureset.StoreOperation = RTC_STOREOPERATION_RESET;

	datestructureset.Year = datestructureget.Year;
	datestructureset.Month = datestructureget.Month;
	datestructureset.Date = datestructureget.Date;
	datestructureset.WeekDay = datestructureget.WeekDay;

	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, FORMAT_BIN);
	HAL_RTC_SetDate(&RTC_Handle, &datestructureset, FORMAT_BIN);
}

/*
 * set minutes -1
 */
void led_clock_minute_minus(){
	RTC_TimeTypeDef timestructureset;
	RTC_DateTypeDef datestructureset;
	RTC_TimeTypeDef timestructureget;
	RTC_DateTypeDef datestructureget;
	HAL_RTC_GetTime(&RTC_Handle, &timestructureget, FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_Handle, &datestructureget, FORMAT_BIN);

	timestructureset.Hours = timestructureget.Hours;
	if(timestructureget.Minutes - 0x01 > 0x00){
		timestructureset.Minutes = timestructureget.Minutes - 0x01;
	}else{
		timestructureset.Minutes = 0x3b;
	}
	timestructureset.Seconds = timestructureget.Seconds;
	timestructureset.TimeFormat = RTC_HOURFORMAT_24;
	timestructureset.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	timestructureset.StoreOperation = RTC_STOREOPERATION_RESET;

	datestructureset.Year = datestructureget.Year;
	datestructureset.Month = datestructureget.Month;
	datestructureset.Date = datestructureget.Date;
	datestructureset.WeekDay = datestructureget.WeekDay;

	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, FORMAT_BIN);
	HAL_RTC_SetDate(&RTC_Handle, &datestructureset, FORMAT_BIN);
}
