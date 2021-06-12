/*
 * webusb_ws2812_board.c
 *
 *  Created on: 15 Nov 2020
 *      Author: Sicris
 */

#include "../tinyusb/hw/bsp/board.h"
#include "stm32f3xx_hal.h"
#include "webusb_ws2812_config.h"

#if (CFG_TUSB_DEBUG >= 2)
static UART_HandleTypeDef huart1;
#endif

// Specific to Tiny USB --->
//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+

void USB_HP_IRQHandler(void)
{
  tud_int_handler(0);
}

void USB_LP_IRQHandler(void)
{
  tud_int_handler(0);
}

void USBWakeUp_RMP_IRQHandler(void)
{
  tud_int_handler(0);
}
// <-- Specific to Tiny USB

// Specific to WS2812 -->
// <-- Specific to WS2812


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV                     = 1
  *            PLLMUL                         = RCC_PLL_MUL9 (9)
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit = {0};

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
  SystemCoreClockUpdate();

  /* Configures the USB clock */
  HAL_RCCEx_GetPeriphCLKConfig(&RCC_PeriphClkInit);
#if (CFG_TUSB_DEBUG >= 2)
  RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1;
  RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
#else
  RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
#endif
  RCC_PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  // Enable USB clock
  __HAL_RCC_USB_CLK_ENABLE();
}

void board_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    SystemClock_Config();

#if CFG_TUSB_OS  == OPT_OS_NONE
    // 1ms tick timer
    SysTick_Config(SystemCoreClock / 1000);
#endif

#ifdef SYSCFG_CFGR1_USB_IT_RMP
    // Remap the USB interrupts
    __HAL_REMAPINTERRUPT_USB_ENABLE();

#if (CFG_TUSB_OS == OPT_OS_FREERTOS)
    NVIC_SetPriority(USB_HP_IRQn, NVIC_PRIORITY_USB );
    NVIC_SetPriority(USB_LP_IRQn, NVIC_PRIORITY_USB );
    NVIC_SetPriority(USBWakeUp_RMP_IRQn, NVIC_PRIORITY_USB );
#endif /* CFG_TUSB_OS == OPT_OS_FREERTOS */

#else

#if (CFG_TUSB_OS == OPT_OS_FREERTOS)
    NVIC_SetPriority(USB_HP_CAN_TX_IRQn, NVIC_PRIORITY_USB );
    NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, NVIC_PRIORITY_USB );
    NVIC_SetPriority(USBWakeUp_IRQn, NVIC_PRIORITY_USB );
#endif /* CFG_TUSB_OS == OPT_OS_FREERTOS */

#endif /* SYSCFG_CFGR1_USB_IT_RMP */

#if (CFG_TUSB_DEBUG >= 2)
    /* Configure UART */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&huart1);
#endif

    /* Configure USB DM and DP pins */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_USB;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USB Pull resistor */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 0);

    /* WS2812 Data output */
    WS2812B_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin       = WS2812B_GPIO_PIN(WS2812B_PIN);
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(WS2812B_PORT, &GPIO_InitStruct);

    // Enable output pins for debuging to see DMA Full and Half transfer interrupts
#if defined(LED4_PORT) && defined(LED5_PORT)
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = LED4_PIN;
    HAL_GPIO_Init(LED4_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LED5_PIN;
    HAL_GPIO_Init(LED5_PORT, &GPIO_InitStruct);
#endif

}

//--------------------------------------------------------------------+
// Board porting API
//--------------------------------------------------------------------+

void board_dplus_pull_up(bool state)
{
    if(state) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 1);
    } else {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, 0);
    }
}

void dcd_connect(uint8_t rhport)
{
    (void)rhport;
    board_dplus_pull_up(true);
}

void dcd_disconnect(uint8_t rhport)
{
    (void)rhport;
    board_dplus_pull_up(false);
}

void board_led_write(bool state)
{
    (void)state;
}

uint32_t board_button_read(void)
{
    return 0;
}

int board_uart_read(uint8_t* buf, int len)
{
    (void) buf; (void) len;
    return 0;
}

int board_uart_write(void const * buf, int len)
{
#if (CFG_TUSB_DEBUG >= 2)
    HAL_UART_Transmit(&huart1, (uint8_t*) buf, len, 0xffff);
    return len;
#else
    (void) buf; (void) len;
    return 0;
#endif /* CFG_TUSB_DEBUG >= 2 */
}

#if CFG_TUSB_OS  == OPT_OS_NONE
volatile uint32_t system_ticks = 0;
void SysTick_Handler (void)
{
    system_ticks++;
}

uint32_t board_millis(void)
{
    return system_ticks;
}
#endif

void HardFault_Handler (void)
{
    asm("bkpt");
}

// Required by __libc_init_array in startup code if we are compiling using
// -nostdlib/-nostartfiles.
void _init(void)
{

}
