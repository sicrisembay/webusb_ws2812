/*

  WS2812B CPU and memory efficient library

  Date: 28.9.2016

  Author: Martin Hubacek
  	  	  http://www.martinhubacek.cz
  	  	  @hubmartin

  Licence: MIT License

*/

#ifndef WS2812B_H_
#define WS2812B_H_

#include "webusb_ws2812_config.h"
#include "stm32f3xx_hal.h"

typedef union {
    uint8_t raw[3];
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
    };
} ws2812_color_t;

// Public functions
// ****************
void ws2812b_init(void);
void ws2812b_write_color(uint32_t id, ws2812_color_t color);

#endif /* WS2812B_H_ */
