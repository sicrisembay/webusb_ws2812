/*
 * webusb_ws2812_config.h
 *
 *  Created on: 15 Nov 2020
 *      Author: Sicris
 */

#ifndef COMPONENTS_BOARD_WEBUSB_WS2812_CONFIG_H_
#define COMPONENTS_BOARD_WEBUSB_WS2812_CONFIG_H_

#include "FreeRTOSConfig.h"
#include "stm32f3xx.h"

#define NVIC_PRIORITY_WS2812    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#define NVIC_PRIORITY_USB       (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)

//=============================================================================
// WS2812 Component Configuration
//=============================================================================
#define WS2812B_REFRESH_RATE_HZ     (30)
// GPIO enable command
#define WS2812B_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
// LED output port
#define WS2812B_PORT                GPIOB
#define WS2812B_PIN                 6
// LED output pins
#define GPIO_CONCAT(x)              GPIO_PIN_##x
#define WS2812B_GPIO_PIN(x)         GPIO_CONCAT(x)
// How many LEDs are in the series
#define WS2812B_NUMBER_OF_LEDS      (24)

// Number of output LED strips. Each has its own buffer.
#define WS2812_BUFFER_COUNT         (1)

// Bit band stuff
#define RAM_BASE 0x20000000
#define RAM_BB_BASE 0x22000000
#define Var_ResetBit_BB(VarAddr, BitNumber) (*(volatile uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0)
#define Var_SetBit_BB(VarAddr, BitNumber) (*(volatile uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1)
#define Var_GetBit_BB(VarAddr, BitNumber) (*(volatile uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)))
#define BITBAND_SRAM(address, bit) ( (__IO uint32_t *) (RAM_BB_BASE + (((uint32_t)address) - RAM_BASE) * 32 + (bit) * 4))

#define varSetBit(var,bit) (Var_SetBit_BB((uint32_t)&var,bit))
#define varResetBit(var,bit) (Var_ResetBit_BB((uint32_t)&var,bit))
#define varGetBit(var,bit) (Var_GetBit_BB((uint32_t)&var,bit))

//=============================================================================
// Parser Component Configuration
//=============================================================================
#define CONFIG_PARSER_RX_BUF_SIZE       (1024)
#define CONFIG_PARSER_TX_BUF_SIZE       (512)
#define CONFIG_CMD_FRAME_SIZE           (64)

#endif /* COMPONENTS_BOARD_WEBUSB_WS2812_CONFIG_H_ */
