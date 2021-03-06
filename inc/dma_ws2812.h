/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void dma_init_ws2812(void);
void sendbuf_WS2812();
void WS2812_framedata_setPixel(uint8_t row, uint16_t column, uint8_t red, uint8_t green, uint8_t blue);
void DMA_SetConfiguration(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
void TIM2_IRQHandlerCall(void);
void DMA2_Stream7_IRQHandlerCall(void);
void TransferComplete(DMA_HandleTypeDef *DmaHandle);
void TransferError(DMA_HandleTypeDef *DmaHandle);
void sendbuf_WS2812(void);
void WS2812_configuration(uint8_t row, uint16_t column);
