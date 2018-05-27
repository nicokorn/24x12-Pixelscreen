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

#include "accelerometer.h"
#include "lis302dl.h"
#include "dma_ws2812.h"
#include "picture_ws2812.h"
#include "stm32f4xx.h"

/* definitions */
static SPI_HandleTypeDef    	SpiHandle;
uint16_t x, y;
extern uint8_t mode;
uint8_t last_mode;
extern uint8_t WS2812_TC;	// global variable: TC=1 transfer completed, TC=0 transfer not completed
static ACCELERO_DrvTypeDef *AcceleroDrv;

/**
  * @brief  Configures the Accelerometer SPI interface.
  */
void accelerometer_io_init(void){
	/* definitions */
	GPIO_InitTypeDef 		GPIO_InitStructure;

	/* Configure the Accelerometer Control pins --------------------------------*/
	/* Enable CS GPIO clock and configure GPIO pin for Accelerometer Chip select */
	__HAL_RCC_GPIOE_CLK_ENABLE();

	/* Configure GPIO PIN for LIS Chip select */
	GPIO_InitStructure.Pin = 	GPIO_PIN_3;
	GPIO_InitStructure.Mode = 	GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull  = 	GPIO_NOPULL;
	GPIO_InitStructure.Speed = 	GPIO_SPEED_MEDIUM;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Deselect: Chip Select high */
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);

	/* SPI configuration */
	if(HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET){
		SpiHandle.Instance = 				SPI1;
	    SpiHandle.Init.BaudRatePrescaler = 	SPI_BAUDRATEPRESCALER_16;
	    SpiHandle.Init.Direction = 			SPI_DIRECTION_2LINES;
	    SpiHandle.Init.CLKPhase = 			SPI_PHASE_1EDGE;
	    SpiHandle.Init.CLKPolarity = 		SPI_POLARITY_LOW;
	    SpiHandle.Init.CRCCalculation = 	SPI_CRCCALCULATION_DISABLED;
	    SpiHandle.Init.CRCPolynomial = 		7;
	    SpiHandle.Init.DataSize = 			SPI_DATASIZE_8BIT;
	    SpiHandle.Init.FirstBit = 			SPI_FIRSTBIT_MSB;
	    SpiHandle.Init.NSS = 				SPI_NSS_SOFT;
	    SpiHandle.Init.TIMode = 			SPI_TIMODE_DISABLED;
	    SpiHandle.Init.Mode = 				SPI_MODE_MASTER;

	    /* Enable the SPI peripheral */
	    __HAL_RCC_SPI1_CLK_ENABLE();

	    /* Enable SCK, MOSI and MISO GPIO clocks */
	    __HAL_RCC_GPIOA_CLK_ENABLE();

	    /* SPI SCK, MOSI, MISO pin configuration */
	    GPIO_InitStructure.Pin = 		(GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	    GPIO_InitStructure.Mode = 		GPIO_MODE_AF_PP;
	    GPIO_InitStructure.Pull  = 		GPIO_PULLDOWN;
	    GPIO_InitStructure.Speed = 		GPIO_SPEED_MEDIUM;
	    GPIO_InitStructure.Alternate = 	GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	    HAL_SPI_Init(&SpiHandle);
	}

	/* Configure GPIO PIN for button */
	GPIO_InitStructure.Pin = 		GPIO_PIN_2;
	GPIO_InitStructure.Mode = 		GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Speed = 		GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Pull  = 		GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

/**
  * @brief  Writes one byte to the Accelerometer.
  * @param  pBuffer: pointer to the buffer containing the data to be written to the Accelerometer.
  * @param  WriteAddr: Accelerometer's internal address to write to.
  * @param  NumByteToWrite: Number of bytes to write.
  */
void ACCELERO_IO_Write(uint8_t *pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite){
	  /* Configure the MS bit:
		 - When 0, the address will remain unchanged in multiple read/write commands.
		 - When 1, the address will be auto incremented in multiple read/write commands.
	  */
	  if(NumByteToWrite > 0x01)
	  {
		WriteAddr |= (uint8_t)MULTIPLEBYTE_CMD;
	  }
	  /* Set chip select Low at the start of the transmission */
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

	  /* Send the Address of the indexed register */
	  SPIx_WriteRead(WriteAddr);

	  /* Send the data that will be written into the device (MSB First) */
	  while(NumByteToWrite >= 0x01){
		SPIx_WriteRead(*pBuffer);
		NumByteToWrite--;
		pBuffer++;
	  }

	  /* Set chip select High at the end of the transmission */
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
}

/**
  * @brief  Reads a block of data from the Accelerometer.
  * @param  pBuffer: pointer to the buffer that receives the data read from the Accelerometer.
  * @param  ReadAddr: Accelerometer's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the Accelerometer.
  */
void ACCELERO_IO_Read(uint8_t *pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead){
	  if(NumByteToRead > 0x01)
	  {
		ReadAddr |= (uint8_t)(READWRITE_CMD | MULTIPLEBYTE_CMD);
	  }
	  else
	  {
		ReadAddr |= (uint8_t)READWRITE_CMD;
	  }
	  /* Set chip select Low at the start of the transmission */
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

	  /* Send the Address of the indexed register */
	  SPIx_WriteRead(ReadAddr);

	  /* Receive the data that will be read from the device (MSB First) */
	  while(NumByteToRead > 0x00)
	  {
		/* Send dummy byte (0x00) to generate the SPI clock to ACCELEROMETER (Slave device) */
		*pBuffer = SPIx_WriteRead(DUMMY_BYTE);
		NumByteToRead--;
		pBuffer++;
	  }

	  /* Set chip select High at the end of the transmission */
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
}

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte: Byte send.
  * @retval The received byte value
  */
uint8_t SPIx_WriteRead(uint8_t Byte){
	  uint8_t receivedbyte = 0;

	  /* Send a Byte through the SPI peripheral */
	  /* Read byte from the SPI bus */
	  if(HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t*) &Byte, (uint8_t*) &receivedbyte, 1, SPIx_TIMEOUT_MAX) != HAL_OK){
		  //Error stay here
	  }

	  return receivedbyte;
}

/**
  * @brief accelerometer mode
  * @param -
  * @retval -
  */
void accelerometer_mode(){
	/* array to save acceleration values */
	volatile int8_t 		x = 0;

    /* read x/y/z acceleration values */
	ACCELERO_IO_Read((uint8_t*)&x, LIS302DL_OUT_Y_ADDR, 1);

	/* calculate the libelle */
	x /= 11;
	x += 11;

	/* processing led output */
	clear_ws2812_leds();
	WS2812_framedata_setPixel(1, x, 0xff, 0xff, 0xff);

	/* send data */
	while(!WS2812_TC);		// wait until the last frame was transmitted
	sendbuf_WS2812();
	/* 10 ms refresh rate */
	HAL_Delay(10);
}

/**
  * @brief  Setx Accelerometer Initialization.
  * @retval ACCELERO_OK if no problem during initialization
  */
uint8_t accelerometer_init(void)
{
  uint8_t ret = 1;
  uint16_t ctrl = 0x0000;
  LIS302DL_InitTypeDef         lis302dl_initstruct;
  LIS302DL_FilterConfigTypeDef lis302dl_filter = {0,0,0};

  if(Lis302dlDrv.ReadID() == I_AM_LIS302DL)
  {
    /* Initialize the accelerometer driver structure */
    AcceleroDrv = &Lis302dlDrv;

    /* Set configuration of LIS302DL MEMS Accelerometer *********************/
    lis302dl_initstruct.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
    lis302dl_initstruct.Output_DataRate = LIS302DL_DATARATE_100;
    lis302dl_initstruct.Axes_Enable = LIS302DL_XYZ_ENABLE;
    lis302dl_initstruct.Full_Scale = LIS302DL_FULLSCALE_2_3;
    lis302dl_initstruct.Self_Test = LIS302DL_SELFTEST_NORMAL;

    /* Configure MEMS: data rate, power mode, full scale, self test and axes */
    ctrl = (uint16_t) (lis302dl_initstruct.Output_DataRate | lis302dl_initstruct.Power_Mode | \
                       lis302dl_initstruct.Full_Scale | lis302dl_initstruct.Self_Test | \
                       lis302dl_initstruct.Axes_Enable);

    /* Configure the accelerometer main parameters */
    AcceleroDrv->Init(ctrl);

    /* MEMS High Pass Filter configuration */
    lis302dl_filter.HighPassFilter_Data_Selection = LIS302DL_FILTEREDDATASELECTION_OUTPUTREGISTER;
    lis302dl_filter.HighPassFilter_CutOff_Frequency = LIS302DL_HIGHPASSFILTER_LEVEL_1;
    lis302dl_filter.HighPassFilter_Interrupt = LIS302DL_HIGHPASSFILTERINTERRUPT_1_2;

    /* Configure MEMS high pass filter cut-off level, interrupt and data selection bits */
    ctrl = (uint8_t)(lis302dl_filter.HighPassFilter_Data_Selection | \
                     lis302dl_filter.HighPassFilter_CutOff_Frequency | \
                     lis302dl_filter.HighPassFilter_Interrupt);

    /* Configure the accelerometer LPF main parameters */
    AcceleroDrv->FilterConfig(ctrl);

    ret = ACCELERO_OK;
  }else{
    ret = ACCELERO_ERROR;
  }
  return ret;
}

/**
  * @brief  Configures the Accelerometer INT2.
  */
void ACCELERO_IO_ITConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable INT2 GPIO clock and configure GPIO PINs to detect Interrupts */
	__HAL_RCC_GPIOE_CLK_ENABLE();

	/* Configure GPIO PIN for button */
	GPIO_InitStructure.Pin = 	GPIO_PIN_2;
	GPIO_InitStructure.Mode = 	GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pull  = 	GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

/**
  * @brief  Read ID of Accelerometer component.
  * @retval ID
  */
uint8_t BSP_ACCELERO_ReadID(void){
	uint8_t id = 0x00;
	if(AcceleroDrv->ReadID != NULL){
		id = AcceleroDrv->ReadID();
	}
	return id;
}

/**
  * @brief  Reboot memory content of Accelerometer.
  */
void BSP_ACCELERO_Reset(void){
	if(AcceleroDrv->Reset != NULL){
		AcceleroDrv->Reset();
	}
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_2){
		if(mode == 0x02){
			mode = last_mode;
		}else{
			last_mode = mode;
			mode = 0x02;
		}
	}
}
