/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACCELEROMETER_H
#define __ACCELEROMETER_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
uint8_t accelerometer_init(void);
void accelerometer_io_init(void);
void ACCELERO_IO_ITConfig(void);
uint8_t  SPIx_WriteRead(uint8_t Byte);
uint8_t BSP_ACCELERO_Init(void);
uint8_t BSP_ACCELERO_ReadID(void);
void    BSP_ACCELERO_Reset(void);

/* Read/Write command */
#define READWRITE_CMD                     ((uint8_t)0x80)

/* Multiple byte read/write command */
#define MULTIPLEBYTE_CMD                  ((uint8_t)0x40)

/* Dummy Byte Send by the SPI Master device in order to generate the Clock to the Slave device */
#define DUMMY_BYTE                        ((uint8_t)0x00)

/* Chip Select macro definition */
#define ACCELERO_CS_LOW()       HAL_GPIO_WritePin(ACCELERO_CS_GPIO_PORT, ACCELERO_CS_PIN, GPIO_PIN_RESET)
#define ACCELERO_CS_HIGH()      HAL_GPIO_WritePin(ACCELERO_CS_GPIO_PORT, ACCELERO_CS_PIN, GPIO_PIN_SET)

/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define SPIx_TIMEOUT_MAX                            0x1000 /*<! The value of the maximal timeout for BUS waiting loops */

/** @defgroup ACCELERO_Driver_structure  Accelerometer Driver structure
  * @{
  */
typedef struct
{
  void      (*Init)(uint16_t);
  void      (*DeInit)(void);
  uint8_t   (*ReadID)(void);
  void      (*Reset)(void);
  void      (*LowPower)(void);
  void      (*ConfigIT)(void);
  void      (*EnableIT)(uint8_t);
  void      (*DisableIT)(uint8_t);
  uint8_t   (*ITStatus)(uint16_t);
  void      (*ClearIT)(void);
  void      (*FilterConfig)(uint8_t);
  void      (*FilterCmd)(uint8_t);
  void      (*GetXYZ)(int16_t *);
}ACCELERO_DrvTypeDef;
/**
  * @}
  */

/** @defgroup ACCELERO_Configuration_structure  Accelerometer Configuration structure
  * @{
  */

/* ACCELERO struct */
typedef struct
{
  uint8_t Power_Mode;                         /* Power-down/Normal Mode */
  uint8_t AccOutput_DataRate;                 /* OUT data rate */
  uint8_t Axes_Enable;                        /* Axes enable */
  uint8_t High_Resolution;                    /* High Resolution enabling/disabling */
  uint8_t BlockData_Update;                   /* Block Data Update */
  uint8_t Endianness;                         /* Endian Data selection */
  uint8_t AccFull_Scale;                      /* Full Scale selection */
  uint8_t Communication_Mode;
}ACCELERO_InitTypeDef;

/* ACCELERO High Pass Filter struct */
typedef struct
{
  uint8_t HighPassFilter_Mode_Selection;      /* Internal filter mode */
  uint8_t HighPassFilter_CutOff_Frequency;    /* High pass filter cut-off frequency */
  uint8_t HighPassFilter_AOI1;                /* HPF_enabling/disabling for AOI function on interrupt 1 */
  uint8_t HighPassFilter_AOI2;                /* HPF_enabling/disabling for AOI function on interrupt 2 */
  uint8_t HighPassFilter_Data_Sel;
  uint8_t HighPassFilter_Stat;
}ACCELERO_FilterConfigTypeDef;

/** @defgroup STM32F4_DISCOVERY_ACCELEROMETER_Exported_Types STM32F4 DISCOVERY ACCELEROMETER Exported Types
  * @{
  */
typedef enum{
  ACCELERO_OK = 0,
  ACCELERO_ERROR = 1,
  ACCELERO_TIMEOUT = 2
}ACCELERO_StatusTypeDef;

#endif /* __LIS302DL_H */
