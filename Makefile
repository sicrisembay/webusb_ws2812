BOARD := stm32f302custom
COMPONENTS_PATH := $(shell realpath components)
TINYUSB_PATH := $(COMPONENTS_PATH)/tinyusb

include $(TINYUSB_PATH)/tools/top.mk
include $(TINYUSB_PATH)/examples/make.mk

FREERTOS_SRC = lib/FreeRTOS/FreeRTOS/Source

INC += \
	main \
	main/usb_device \
	main/parser \
	$(COMPONENTS_PATH)/ws2812b \
	$(COMPONENTS_PATH)/board \
	$(TOP)/hw \
	$(TINYUSB_PATH)/hw/bsp/$(BOARD) \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/include \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/portable/GCC/$(FREERTOS_PORT)
	
# Main source
SRC_C += \
	main/main.c \
	main/freertos_hook.c \
	main/usb_device/usb_descriptors.c \
	main/usb_device/webusb.c \
	main/parser/commandParser.c \
	main/parser/frameParser.c \
	$(COMPONENTS_PATH)/ws2812b/ws2812b.c

# FreeRTOS source, all files in port folder
SRC_C += \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/list.c \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/queue.c \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/tasks.c \
	$(TINYUSB_PATH)/$(FREERTOS_SRC)/timers.c \
	$(subst components/tinyusb/,,$(wildcard components/tinyusb/$(FREERTOS_SRC)/portable/GCC/$(FREERTOS_PORT)/*.c))

# FreeRTOS (lto + Os) linker issue
LDFLAGS += -Wl,--undefined=vTaskSwitchContext
CFLAGS += -Wno-error=discarded-qualifiers

include $(TINYUSB_PATH)/examples/rules.mk
