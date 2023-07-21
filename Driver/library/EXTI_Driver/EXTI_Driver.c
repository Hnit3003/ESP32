#include "EXTI_Driver.h"

xQueueHandle xQueue_ExternalInterrupt;

static void IRAM_ATTR external_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(xQueue_ExternalInterrupt, &pinNumber, NULL);
}

void external_interrupt_task(void *pvParameter)
{
    int EXTI_Pin = 0;
    while(1)
    {
        if(xQueueReceive(xQueue_ExternalInterrupt, &EXTI_Pin, portMAX_DELAY))
        {   
            vTaskDelay(20 / portTICK_PERIOD_MS);
            xQueueReset(xQueue_ExternalInterrupt);
            if(gpio_get_level(EXTI_Pin) == 1)
            {
                continue;
            }

            //Function handle EXTI_Source
        }
    }
}

void external_interrupt_initialize(gpio_num_t gpio_pin, gpio_int_type_t intr_edge)
{
    xQueue_ExternalInterrupt = xQueueCreate(10, sizeof(int));
    TaskHandle_t xEXTI_TASK = NULL;
    xTaskCreate(external_interrupt_task, "EXTI_Task", 2048, NULL, 1, &xEXTI_TASK);
    
    gpio_config_t pin_conf;
    pin_conf.pin_bit_mask = 1ULL << gpio_pin;
    pin_conf.mode = GPIO_MODE_INPUT;
    pin_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pin_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    pin_conf.intr_type = intr_edge;
    gpio_config(&pin_conf);   

    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_pin, external_interrupt_handler, (void *)gpio_pin); 
}
