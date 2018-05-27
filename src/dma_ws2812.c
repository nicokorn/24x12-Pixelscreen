/*
 * Autor: Nico Korn
 * Date: 26.10.2017
 * Firmware for the STM32F4Discovery Board to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * dma_ws2812.c this module contents the initialization of the dma transfer with framebuffer handling.
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
#include "dma_ws2812.h"
#include "stm32f4xx.h"

//defines
/* this define sets the number of TIM1 overflows
 * to append to the data frame for the LEDs to
 * load the received data into their registers */
#define WS2812_DEADPERIOD 19
/* WS2812 framebuffer
 * buffersize = (#LEDs / 16) * 24 */
#define BUFFERSIZE 576 //old with 5 leds 120

//variables
extern TIM_HandleTypeDef TIM_HandleStruct;
uint8_t WS2812_TC;
DMA_HandleTypeDef DMA_HandleStruct_UEV;
DMA_HandleTypeDef DMA_HandleStruct_CC1;
DMA_HandleTypeDef DMA_HandleStruct_CC2;
uint16_t WS2812_IO_High = 0xFFFF;
uint16_t WS2812_IO_Low = 0x0000;
uint16_t WS2812_IO_framedata[BUFFERSIZE];
uint8_t TIM1_overflows = 0;
uint8_t dma_tc_flag = 0;
uint16_t transfer_counter = 0;

//function
void dma_init_ws2812(void){

	__HAL_RCC_DMA2_CLK_ENABLE();				//activate bus on which dma2 is connected

	// TIM1 Update event, High Output
	/* DMA2 Channel6 configuration ----------------------------------------------*/
	DMA_HandleStruct_UEV.Instance 					= DMA2_Stream5;
	DMA_HandleStruct_UEV.Init.Channel 				= DMA_CHANNEL_6;
	DMA_HandleStruct_UEV.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
	DMA_HandleStruct_UEV.Init.PeriphInc 			= DMA_PINC_DISABLE;
	DMA_HandleStruct_UEV.Init.MemInc 				= DMA_MINC_DISABLE;
	DMA_HandleStruct_UEV.Init.Mode 					= DMA_NORMAL;
	DMA_HandleStruct_UEV.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_HALFWORD;
	DMA_HandleStruct_UEV.Init.MemDataAlignment 		= DMA_MDATAALIGN_HALFWORD;
	DMA_HandleStruct_UEV.Init.Priority 				= DMA_PRIORITY_HIGH;
	DMA_HandleStruct_UEV.Init.FIFOMode 				= DMA_FIFOMODE_DISABLE;
	HAL_DMA_DeInit(&DMA_HandleStruct_UEV);
	DMA_SetConfiguration(&DMA_HandleStruct_UEV, (uint32_t)WS2812_IO_High, (uint32_t)&GPIOC->ODR, BUFFERSIZE);
	if(HAL_DMA_Init(&DMA_HandleStruct_UEV) != HAL_OK){
		while(1){
			//error
		}
	}

	// TIM1 CC1 event, Dataframe Output, needs bit incrementation on memory
	/* DMA2 Channel6 configuration ----------------------------------------------*/
	DMA_HandleStruct_CC1.Instance 					= DMA2_Stream1;
	DMA_HandleStruct_CC1.Init.Channel 				= DMA_CHANNEL_6;
	DMA_HandleStruct_CC1.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
	DMA_HandleStruct_CC1.Init.PeriphInc 			= DMA_PINC_DISABLE;
	DMA_HandleStruct_CC1.Init.MemInc 				= DMA_MINC_ENABLE;
	DMA_HandleStruct_CC1.Init.Mode 					= DMA_NORMAL;
	DMA_HandleStruct_CC1.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC1.Init.MemDataAlignment 		= DMA_MDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC1.Init.Priority 				= DMA_PRIORITY_HIGH;
	DMA_HandleStruct_CC1.Init.FIFOMode 				= DMA_FIFOMODE_DISABLE;
	HAL_DMA_DeInit(&DMA_HandleStruct_CC1);
	DMA_SetConfiguration(&DMA_HandleStruct_CC1, (uint32_t)WS2812_IO_framedata, (uint32_t)&GPIOC->ODR, BUFFERSIZE);
	if(HAL_DMA_Init(&DMA_HandleStruct_CC1) != HAL_OK){
		while(1){
			//error
		}
	}

	// TIM1 CC2 event, Low Output
	/* DMA2 Channel6 configuration ----------------------------------------------*/
	DMA_HandleStruct_CC2.Instance 					= DMA2_Stream2;
	DMA_HandleStruct_CC2.Init.Channel 				= DMA_CHANNEL_6;
	DMA_HandleStruct_CC2.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
	DMA_HandleStruct_CC2.Init.PeriphInc 			= DMA_PINC_DISABLE;
	DMA_HandleStruct_CC2.Init.MemInc 				= DMA_MINC_DISABLE;
	DMA_HandleStruct_CC2.Init.Mode 					= DMA_NORMAL;
	DMA_HandleStruct_CC2.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC2.Init.MemDataAlignment 		= DMA_MDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC2.Init.Priority 				= DMA_PRIORITY_HIGH;
	DMA_HandleStruct_CC2.Init.FIFOMode 				= DMA_FIFOMODE_DISABLE;
	HAL_DMA_DeInit(&DMA_HandleStruct_CC2);
	DMA_SetConfiguration(&DMA_HandleStruct_CC2, (uint32_t)WS2812_IO_Low, (uint32_t)&GPIOC->ODR, BUFFERSIZE);
	if(HAL_DMA_Init(&DMA_HandleStruct_CC2) != HAL_OK){
		while(1){
			//error
		}
	}

	//register callbacks
	HAL_DMA_RegisterCallback(&DMA_HandleStruct_CC2, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
	HAL_DMA_RegisterCallback(&DMA_HandleStruct_CC2, HAL_DMA_XFER_ERROR_CB_ID, TransferError);

	/* NVIC configuration for DMA transfer complete interrupt */
	HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 1, 1);
	/* Enable interrupt */
	HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
}

void sendbuf_WS2812(){

	// transmission complete flag, indicate that transmission is taking place
	WS2812_TC = 0;

	// clear all relevant DMA flags from the streams 1,2 and 5
	__HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_UEV, DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 | DMA_FLAG_TEIF1_5 | DMA_FLAG_DMEIF1_5 | DMA_FLAG_FEIF1_5);
	__HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_CC1, DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 | DMA_FLAG_TEIF1_5 | DMA_FLAG_DMEIF1_5 | DMA_FLAG_FEIF1_5);
	__HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_CC2, DMA_FLAG_TCIF2_6 | DMA_FLAG_HTIF2_6 | DMA_FLAG_TEIF2_6 | DMA_FLAG_DMEIF2_6 | DMA_FLAG_FEIF2_6);

    /* Enable the selected DMA transfer interrupts */
	DMA_HandleStruct_CC2.Instance->CR  |= DMA_IT_TC | DMA_IT_TE | DMA_IT_DME;
	DMA_HandleStruct_CC2.Instance->FCR |= DMA_IT_FE;
    if(DMA_HandleStruct_CC2.XferHalfCpltCallback != NULL)
    {
    	DMA_HandleStruct_CC2.Instance->CR  |= DMA_IT_HT;
    }

	// enable dma channels
	__HAL_DMA_ENABLE(&DMA_HandleStruct_UEV);
	__HAL_DMA_ENABLE(&DMA_HandleStruct_CC1);
	__HAL_DMA_ENABLE(&DMA_HandleStruct_CC2);

	// clear all TIM1 flags
	TIM1->SR = 0;

	// IMPORTANT: enable the TIM1 DMA requests AFTER enabling the DMA channels!
	__HAL_TIM_ENABLE_DMA(&TIM_HandleStruct, TIM_DMA_UPDATE);
	__HAL_TIM_ENABLE_DMA(&TIM_HandleStruct, TIM_DMA_CC1);
	__HAL_TIM_ENABLE_DMA(&TIM_HandleStruct, TIM_DMA_CC2);

	// Enable the Output compare channel
	TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_1, TIM_CCx_ENABLE);
	TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_2, TIM_CCx_ENABLE);

	// preload counter with 29 so TIM1 generates UEV directly to start DMA transfer
	__HAL_TIM_SET_COUNTER(&TIM_HandleStruct, 29);

	// start TIM1
	__HAL_TIM_ENABLE(&TIM_HandleStruct);
}

/* DMA2 Channel7 Interrupt Handler gets executed once the complete framebuffer
 * has been transmitted to the LEDs *///DMA2_Stream0_IRQHandler
void DMA2_Stream2_IRQHandler(void){
	// set irq handler
	HAL_DMA_IRQHandler(&DMA_HandleStruct_CC2);
}


/* TIM1 Interrupt Handler gets executed on every TIM1 Update if enabled */
void TIM1_UP_TIM10_IRQHandler(void){
	// Clear TIM1 Interrupt Flag
	HAL_NVIC_ClearPendingIRQ(TIM1_UP_TIM10_IRQn);

	/* check if certain number of overflows has occured yet
	 * this ISR is used to guarantee a 19*1.25 us = 23.75 us dead time on the data lines (measured 28.8 us)
	 * before another frame is transmitted */
	if (TIM1_overflows < (uint8_t)WS2812_DEADPERIOD){
		// count the number of occured overflows
		TIM1_overflows++;
	}else{
		// clear the number of overflows
		TIM1_overflows = 0;

		// stop TIM1 now because dead period has been reached
		TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_1, TIM_CCx_DISABLE);
		TIM_CCxChannelCmd(TIM1, TIM_CHANNEL_2, TIM_CCx_DISABLE);
		__HAL_TIM_DISABLE(&TIM_HandleStruct);

		// disable the TIM1 Update interrupt again so it doesn't occur while transmitting data
		__HAL_TIM_DISABLE_IT(&TIM_HandleStruct, TIM_IT_UPDATE);

		// finally indicate that the data frame has been transmitted
		WS2812_TC = 1;
	}
}

/* This function sets the color of a single pixel in the framebuffer
 *
 * Arguments:
 * row = the channel number/LED strip the pixel is in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixel should display
 */
void WS2812_framedata_setPixel(uint8_t row, uint16_t column, uint8_t red, uint8_t green, uint8_t blue){

	uint8_t i;

	for (i = 0; i < 8; i++){
		// clear the data for pixel
		WS2812_IO_framedata[((column*24)+i)] &= ~(0x01<<row);
		WS2812_IO_framedata[((column*24)+8+i)] &= ~(0x01<<row);
		WS2812_IO_framedata[((column*24)+16+i)] &= ~(0x01<<row);

		// write new data for pixel
		WS2812_IO_framedata[((column*24)+i)] |= ((((green<<i) & 0x80)>>7)<<row);
		WS2812_IO_framedata[((column*24)+8+i)] |= ((((red<<i) & 0x80)>>7)<<row);
		WS2812_IO_framedata[((column*24)+16+i)] |= ((((blue<<i) & 0x80)>>7)<<row);
	}
}

/* This function sets the amount of leds and its rows and columns configuration
 *
 * Arguments:
 * row = the channel number/LED strip the pixel is in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 */
void WS2812_configuration(uint8_t row, uint16_t column){

	//uint8_t buffersize;
	//* buffersize = (#LEDs / 16) * 24 */
	//uint16_t WS2812_IO_framedata[BUFFERSIZE];

}

/**
  * @brief  Sets the DMA Transfer parameter.
  * @param  hdma:       pointer to a DMA_HandleTypeDef structure that contains
  *                     the configuration information for the specified DMA Stream.
  * @param  SrcAddress: The source memory Buffer address
  * @param  DstAddress: The destination memory Buffer address
  * @param  DataLength: The length of data to be transferred from source to destination
  * @retval HAL status
  */
void DMA_SetConfiguration(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength){
  /* Clear DBM bit */
  hdma->Instance->CR &= (uint32_t)(~DMA_SxCR_DBM);

  /* Configure DMA Stream data length */
  hdma->Instance->NDTR = DataLength;

  /* Peripheral to Memory */
  if((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH)
  {
    /* Configure DMA Stream destination address */
    hdma->Instance->PAR = DstAddress;

    /* Configure DMA Stream source address */
    hdma->Instance->M0AR = SrcAddress;
  }
  /* Memory to Peripheral */
  else
  {
    /* Configure DMA Stream source address */
    hdma->Instance->PAR = SrcAddress;

    /* Configure DMA Stream destination address */
    hdma->Instance->M0AR = DstAddress;
  }
}

/**
  * @brief  DMA conversion complete callback
  * @note   This function is executed when the transfer complete interrupt
  *         is generated
  * @retval None
  */
void TransferComplete(DMA_HandleTypeDef *DmaHandle){
	// clear DMA7 transfer complete interrupt flag
	HAL_NVIC_ClearPendingIRQ(DMA2_Stream2_IRQn);

	// enable TIM1 Update interrupt to append 50us dead period
	__HAL_TIM_ENABLE_IT(&TIM_HandleStruct, TIM_IT_UPDATE);

	// disable the DMA channels
	__HAL_DMA_DISABLE(&DMA_HandleStruct_UEV);
	__HAL_DMA_DISABLE(&DMA_HandleStruct_CC1);
	__HAL_DMA_DISABLE(&DMA_HandleStruct_CC2);

	// IMPORTANT: disable the DMA requests, too!
	__HAL_TIM_DISABLE_DMA(&TIM_HandleStruct, TIM_DMA_UPDATE);
	__HAL_TIM_DISABLE_DMA(&TIM_HandleStruct, TIM_DMA_CC1);
	__HAL_TIM_DISABLE_DMA(&TIM_HandleStruct, TIM_DMA_CC2);
}

/**
  * @brief  DMA conversion error callback
  * @note   This function is executed when the transfer error interrupt
  *         is generated during DMA transfer
  * @retval None
  */
void TransferError(DMA_HandleTypeDef *DmaHandle){
	while(1){

	}
}
