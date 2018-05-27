/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void usart_init_bt(void);
void HAL_USART_RxCpltCallback(USART_HandleTypeDef *husart);
void get_rx_data_buffer(uint8_t *rx_buffer);

