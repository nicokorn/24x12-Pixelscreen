/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
   uint32_t color;
   uint8_t red;
   uint8_t green;
   uint8_t blue;
   uint16_t x;
   uint16_t y;
   uint16_t x_arr[12];
   uint16_t y_arr[12];
   uint16_t counter;
} fx_struct;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void led_knight_rider(void);
void color_fading(void);
void clear_ws2812_leds(void);
void led_test(void);
void set_ws2812_leds_background(void);
void picture_load(uint8_t picture_array[], uint16_t array_length, uint16_t col_count, uint16_t row_count);
void snow_drop(void);
void fx_init(void);
void fx_chooser(uint8_t index);
