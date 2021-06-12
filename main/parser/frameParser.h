#ifndef _FRAME_PARSER_H_
#define _FRAME_PARSER_H_

#include "stdint.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef int32_t (*callbacValidFrame)(uint32_t);

typedef struct {
    callbacValidFrame callback;
    uint8_t * pCommandBuffer;
    SemaphoreHandle_t mutex;
} frame_valid_cb_t;

void frame_parser_init(frame_valid_cb_t * pCallbackDef);
void frame_parser_receive(uint8_t *pBuf, uint32_t len);
void frame_parser_process(void);
void frame_parser_format_frame(uint8_t *pBuf, uint32_t len);

#endif /* _FRAME_PARSER_H_ */
