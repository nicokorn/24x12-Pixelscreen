/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/

#include "gpio_ws2812.h"
#include "timer_ws2812.h"
#include "dma_ws2812.h"
#include "picture_ws2812.h"
#include "usart_bt.h"
#include <stdio.h>
#include <stdlib.h>




/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void SystemClock_Config(void);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
