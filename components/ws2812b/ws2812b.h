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
#include "stm32f3xx_hal_dma.h"

// Library structures
// ******************
// This value sets number of periods to generate 50uS Treset signal
#define WS2812_RESET_PERIOD 12

typedef struct WS2812_BufferItem {
	uint8_t* frameBufferPointer;
	uint32_t frameBufferSize;
	uint32_t frameBufferCounter;
	uint8_t channel;	// digital output pin/channel
} WS2812_BufferItem;



typedef struct WS2812_Struct
{
	WS2812_BufferItem item[WS2812_BUFFER_COUNT];
	uint8_t transferComplete;
	uint8_t startTransfer;
	uint32_t timerPeriodCounter;
	uint32_t repeatCounter;
} WS2812_Struct;

// Public functions
// ****************
void ws2812b_init();

#endif /* WS2812B_H_ */
