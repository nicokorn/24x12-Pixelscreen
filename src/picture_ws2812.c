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

#include "picture_ws2812.h"
#include "stm32f4xx.h"

//defines
#define COLS 24
#define ROWS 12

//variables
extern uint8_t WS2812_TC = 1;	// global variable: TC=1 transfer completed, TC=0 transfer not completed
/* structs for effects definitions */
fx_struct st_color_fading;
fx_struct st_snow_drop;
fx_struct st_led_test;

//arrays
/* Array defining 12 color triplets to be displayed */
uint8_t colors[12][3] = {
	{0xFF, 0x00, 0x00},	// red
	{0xFF, 0x80, 0x00},	// orange
	{0xFF, 0xFF, 0x00}, // yellow
	{0x80, 0xFF, 0x00},
	{0x00, 0xFF, 0x00}, // green
	{0x00, 0xFF, 0x80},
	{0x00, 0xFF, 0xFF},
	{0x00, 0x80, 0xFF},
	{0x00, 0x00, 0xFF},
	{0x80, 0x00, 0xFF},
	{0xFF, 0x00, 0xFF},
	{0xFF, 0x00, 0x80}
};

/* init function for variables of the effects */
void fx_init(){
	/* color fading */
	st_color_fading.color = 0x00000000;
	st_color_fading.red = 0x00;
	st_color_fading.green = 0x00;
	st_color_fading.blue = 0xff;
	/* snow drop */
	st_snow_drop.color = 0x00000000;
	st_snow_drop.red = 0x00;
	st_snow_drop.green = 0x00;
	st_snow_drop.blue = 0x00;
	for(int i = 0; i<12; i++){
		st_snow_drop.x_arr[i] = (rand() % 24);
		st_snow_drop.y_arr[i] = (rand() % 12);
	}
	st_snow_drop.counter = 0;
	/* led test */
	st_led_test.color = 0x00000000;
	st_led_test.red = 0x00;
	st_led_test.green = 0x00;
	st_led_test.blue = 0x00;
	st_led_test.x = 0;
	st_led_test.y = 0;
	st_led_test.counter = 0;

}

void fx_chooser(uint8_t index){
	index %= 4;
	switch(index){
		case 0:		color_fading();
		break;
		case 1: 	snow_drop();
		break;
		case 2: 	led_test();
		break;
		case 3:		set_ws2812_leds_background();
					while(!WS2812_TC);	// wait until the last frame was transmitted
					sendbuf_WS2812();	// send the framebuffer out to the LEDs
					HAL_Delay(50);		// 50 ms refresh rate
		break;
	}
}

/* with this function all leds will start to fade color together */
void color_fading(){
	//coordinates
	uint16_t x,y;

	// set two pixels (columns) in the defined row (channel 0) to the
	// color values defined in the colors array
	for (y = 0; y < ROWS; y++){
		for(x = 0; x < COLS; x++){
			WS2812_framedata_setPixel(y, x, st_color_fading.red, st_color_fading.green, st_color_fading.blue);
		}
	}
	// wait until the last frame was transmitted
	while(!WS2812_TC);
	// send the framebuffer out to the LEDs
	sendbuf_WS2812();
	// 50 ms refresh rate
	HAL_Delay(50);

	//set next colors
	if(st_color_fading.green == 0x00 && st_color_fading.red < 0xff && st_color_fading.blue >= 0x00){
		st_color_fading.red += 0x01;
		st_color_fading.green = 0x00;
		st_color_fading.blue -= 0x01;
	}else if(st_color_fading.green < 0xff && st_color_fading.red >= 0x00 && st_color_fading.blue == 0x00){
		st_color_fading.red -= 0x01;
		st_color_fading.green += 0x01;
		st_color_fading.blue = 0x00;
	}else if(st_color_fading.green >= 0x00 && st_color_fading.red == 0x00 && st_color_fading.blue < 0xff){
		st_color_fading.red = 0x00;
		st_color_fading.green -= 0x01;
		st_color_fading.blue += 0x01;
	}
}

//function
void snow_drop(){

	/* delete display */
	clear_ws2812_leds();

	/* set snowflakes */
	for(int i=0; i<12; i++){
		if(st_snow_drop.y_arr[i] < 12){
			WS2812_framedata_setPixel(st_snow_drop.y_arr[i], st_snow_drop.x_arr[i], 0xff, 0xff, 0xff);
		}else{
			st_snow_drop.y_arr[i] = 0;
			WS2812_framedata_setPixel(st_snow_drop.y_arr[i], st_snow_drop.x_arr[i], 0xff, 0xff, 0xff);
		}
		st_snow_drop.y_arr[i]++;
		//if(i %= 2){
		//	st_snow_drop.y_arr[i]++;
		//}
	}

	/* send data to leds */
	while(!WS2812_TC);		// wait until the last frame was transmitted
	sendbuf_WS2812();		// send the framebuffer out to the LEDs
	HAL_Delay(25);			// 50 ms refresh rate
}

//function
void clear_ws2812_leds(){
	uint16_t x, y;
	//set all dark
	for(y=0; y<ROWS; y++){
		for(x = 0; x<COLS; x++){
			WS2812_framedata_setPixel(y, x, 0x00, 0x00, 0x00);
		}
	}
}

//function
void set_ws2812_leds_background(){
	uint16_t x, y;
	uint8_t red = 0x00;
	uint8_t green = 0x00;
	uint8_t blue = 0x00;
	//set all dark
	for(y=0; y<ROWS; y++){
		for(x = 0; x<COLS; x++){
			WS2812_framedata_setPixel(y, x, red, green, blue);
		}
		//red+=0x01;
		green+=1;
		//blue+=0x01;
	}
}

//function
void led_test(){
	st_led_test.red = st_led_test.color >> 4;
	st_led_test.green = st_led_test.color >> 2;
	st_led_test.blue = st_led_test.color >> 0;
	//set pixel
	WS2812_framedata_setPixel(st_led_test.y, st_led_test.x, st_led_test.red, st_led_test.green, st_led_test.blue);
	//set coordinates for next dot
	if(st_led_test.x < 23){
		st_led_test.x++;
	}else{
		st_led_test.x=0;
		st_led_test.y++;
	}
	if(st_led_test.y < 12){
		//y++;
	}else{
		st_led_test.y=0;
	}
	// send the framebuffer out to the LEDs
	sendbuf_WS2812();
	// 25 ms refresh rate
	HAL_Delay(10);
	//change color
	st_led_test.color += 0x00000002;
}

//function
void picture_load(uint8_t picture_array[], uint16_t array_length, uint16_t col_count, uint16_t row_count){
	//local variables
	uint16_t x;
	uint16_t y;
	uint16_t i;

	//set picture on led device
	i=0;
	for(y=0; y<row_count; y++){
		for(x=0; x<col_count; x++){
			WS2812_framedata_setPixel(y, x, picture_array[i], picture_array[i+1], picture_array[i+2]);
			i+=3;
		}
	}

	// send the framebuffer out to the LEDs
	sendbuf_WS2812();
	// 25 ms refresh rate
	HAL_Delay(10);
}
