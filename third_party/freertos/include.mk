TARGET_FREERTOS_PATH = $(FREERTOS_MODULE_PATH)
INCLUDE_DIRS += $(FREERTOS_MODULE_PATH)/freertos/FreeRTOS/Source/include

ifneq ("$(PLATFORM_FREERTOS)","")
INCLUDE_DIRS += $(FREERTOS_MODULE_PATH)/freertos/FreeRTOS/Source/portable/GCC/$(PLATFORM_FREERTOS)
endif
