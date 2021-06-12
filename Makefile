BOARD := stm32f302custom
TINYUSB_PATH := $(shell realpath components/tinyusb)

include $(TINYUSB_PATH)/tools/top.mk
include $(TINYUSB_PATH)/examples/make.mk

FREERTOS_SRC = lib/FreeRTOS/FreeRTOS/Source

INC += \
	main \
	$(TOP)/hw \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/include \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/portable/GCC/$(FREERTOS_PORT)
	
# Main source
SRC_C += \
	main/main.c \
	main/freertos_hook.c \
	main/usb_descriptors.c

# FreeRTOS source, all files in port folder
SRC_C += \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/list.c \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/queue.c \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/tasks.c \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/timers.c \
	$(subst components/tinyusb/,,$(wildcard components/tinyusb/$(FREERTOS_SRC)/portable/GCC/$(FREERTOS_PORT)/*.c))

# FreeRTOS (lto + Os) linker issue
LDFLAGS += -Wl,--undefined=vTaskSwitchContext

include $(TINYUSB_PATH)/examples/rules.mk
