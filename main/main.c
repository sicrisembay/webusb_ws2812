/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "ws2812b.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

// static task for usbd
// Increase stack size when debug log is enabled
#if CFG_TUSB_DEBUG
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE)
#else
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE/2)
#endif

StackType_t  usb_device_stack[USBD_STACK_SIZE];
StaticTask_t usb_device_taskdef;

// static task for usb class
#define CLASS_STACK_SIZE      (3*configMINIMAL_STACK_SIZE)
StackType_t  class_stack[CLASS_STACK_SIZE];
StaticTask_t class_taskdef;

#define URL  "www.tinyusb.org/examples/webusb-serial"

const tusb_desc_webusb_url_t desc_url =
{
  .bLength         = 3 + sizeof(URL) - 1,
  .bDescriptorType = 3, // WEBUSB URL type
  .bScheme         = 1, // 0: http, 1: https
  .url             = URL
};

static bool web_serial_connected = false;

void usb_device_task(void* param);
void class_task(void * params);

void webserial_task(void);
void cdc_task(void);

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+

int main(void)
{
    board_init();

    ws2812b_init();

    // Create a task for tinyusb device stack
    (void) xTaskCreateStatic( usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, usb_device_stack, &usb_device_taskdef);

    // Create Class task
    (void) xTaskCreateStatic( class_task, "class", CLASS_STACK_SIZE, NULL, configMAX_PRIORITIES-2, class_stack, &class_taskdef);

    vTaskStartScheduler();
    NVIC_SystemReset();
    return 0;
}

// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
void usb_device_task(void* param)
{
    (void) param;

    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    tusb_init();

    // RTOS forever loop
    while (1)
    {
        // tinyusb device task
        tud_task();
    }
}

// send characters to both CDC and WebUSB
void echo_all(uint8_t buf[], uint32_t count)
{
    // echo to web serial
    if ( web_serial_connected ) {
        tud_vendor_write(buf, count);
    }

    // echo to cdc
    if ( tud_cdc_connected() ) {
        for(uint32_t i=0; i<count; i++) {
            tud_cdc_write_char(buf[i]);
            if ( buf[i] == '\r' ) {
                tud_cdc_write_char('\n');
            }
        }
        tud_cdc_write_flush();
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    /// TODO
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    /// TODO
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    /// TODO
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    /// TODO
}

//--------------------------------------------------------------------+
// WebUSB use vendor class
//--------------------------------------------------------------------+

// Invoked when received VENDOR control request
bool tud_vendor_control_request_cb(uint8_t rhport, tusb_control_request_t const * request)
{
    switch (request->bRequest)
    {
        case VENDOR_REQUEST_WEBUSB:
            // match vendor request in BOS descriptor
            // Get landing page url
            return tud_control_xfer(rhport, request, (void*) &desc_url, desc_url.bLength);

        case VENDOR_REQUEST_MICROSOFT:
            if ( request->wIndex == 7 ) {
                // Get Microsoft OS 2.0 compatible descriptor
                uint16_t total_len;
                memcpy(&total_len, desc_ms_os_20+8, 2);

                return tud_control_xfer(rhport, request, (void*) desc_ms_os_20, total_len);
            } else {
                return false;
            }

        case 0x22:
            // Webserial simulate the CDC_REQUEST_SET_CONTROL_LINE_STATE (0x22) to
            // connect and disconnect.
            web_serial_connected = (request->wValue != 0);

            // response with status OK
            return tud_control_status(rhport, request);

        default:
            // stall unknown request
            return false;
    }

    return true;
}

// Invoked when DATA Stage of VENDOR's request is complete
bool tud_vendor_control_complete_cb(uint8_t rhport, tusb_control_request_t const * request)
{
    (void) rhport;
    (void) request;

    // nothing to do
    return true;
}

void webserial_task(void)
{
    if(web_serial_connected) {
        if ( tud_vendor_available() ) {
            uint8_t buf[64];
            uint32_t count = tud_vendor_read(buf, sizeof(buf));

            // echo back to both web serial and cdc
            echo_all(buf, count);
        }
    }
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
    if ( tud_cdc_connected() ) {
        // connected and there are data available
        if ( tud_cdc_available() ) {
            uint8_t buf[64];

            // read and echo back
            uint32_t count = tud_cdc_read(buf, sizeof(buf));

            // echo back to both web serial and cdc
            echo_all(buf, count);
        }
    }
}

void class_task(void * params)
{
    (void) params;
    
    // RTOS forever loop
    while(1) {
        cdc_task();
        webserial_task();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;

    // connected
    if ( dtr && rts ) {
        // print initial message when connected
        tud_cdc_write_str("\r\nTinyUSB CDC MSC device with FreeRTOS example\r\n");
    }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}
