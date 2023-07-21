#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "GPIO_Driver.h"
#include "EXTI_Driver.h"

static void IRAM_ATTR external_interrupt_handler(void *args);

void app_main(void)
{
    external_interrupt_initialize(GPIO_NUM_5, GPIO_INTR_NEGEDGE);
}
