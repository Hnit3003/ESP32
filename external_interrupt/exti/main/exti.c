#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "gpio.h"

#define LED_PIN 2
#define BUTTON_PIN 5

uint8_t LED_State = 0;
xQueueHandle xQueue_Interrupt;

static void IRAM_ATTR gpio_intterupt_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(xQueue_Interrupt, &pinNumber, NULL);
}

void LED_Control_Task(void *param)
{
    int pinNumber, count = 0;
    while(1)
    {
        if(xQueueReceive(xQueue_Interrupt, &pinNumber, portMAX_DELAY))
        {
            printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(BUTTON_PIN));
            gpio_set_level(LED_PIN, gpio_get_level(BUTTON_PIN));
        }

    }
}

void app_main(void)
{
    gpio_set_output(LED_PIN);
    gpio_set_intr_pullup(BUTTON_PIN);
    
    xQueue_Interrupt = xQueueCreate(10, sizeof(int));
    xTaskCreate(LED_Control_Task, "LED_Control_Task", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, gpio_intterupt_handler, (void *)BUTTON_PIN); 

}

