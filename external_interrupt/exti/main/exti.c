#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "gpio.h"

#define LED_BLINK 2
#define LED_ISR1 19
#define LED_ISR2 21
#define BUTTON_PIN_5 5
#define BUTTON_PIN_15 15

xQueueHandle Queue_EXTI;

uint8_t LED_BLINK_State = 0;
uint16_t countBTN1, countBTN2 = 0;
volatile int queue_count = 0;

static void IRAM_ATTR external_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(Queue_EXTI, &pinNumber, NULL);
    queue_count = (int)uxQueueMessagesWaitingFromISR(Queue_EXTI);
}

void buttonEXTI_Task(void *param)
{
    int pinNumber;
    while(1)
    {
        if(xQueueReceive(Queue_EXTI, &pinNumber, portMAX_DELAY))
        {
            printf("%d", queue_count);
            queue_count = 0;
            // printf("ISR: %d", (int)uxQueueMessagesWaiting(Queue_EXTI));
            vTaskDelay(20 / portTICK_PERIOD_MS);
            xQueueReset(Queue_EXTI);
            printf("-> %d\n", (int)uxQueueMessagesWaiting(Queue_EXTI));
            if(gpio_get_level(pinNumber) == 1)
            {
                continue;  
            }

            switch (pinNumber)
            {
            case BUTTON_PIN_5:
                printf("GPIO %d press: %d times\n", pinNumber, countBTN1++);
                gpio_set_level(LED_ISR1, countBTN1 % 2);
                break;
            
            case BUTTON_PIN_15:
                printf("GPIO %d press: %d times\n", pinNumber, countBTN2++);
                gpio_set_level(LED_ISR2, countBTN2 % 2);
                break;
            
            default:
                break;
            }  
        }
    }
}

void blinkLED_Task(void *param)
{
    while(1)
    {
        LED_BLINK_State = !LED_BLINK_State;
        gpio_set_level(LED_BLINK, LED_BLINK_State);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
} 

void app_main(void)
{
   TaskHandle_t xBUTTON_INTERRUPT, xBLINK_LED = NULL;
    gpio_set_output(LED_BLINK);
    gpio_set_output(LED_ISR1);
    gpio_set_output(LED_ISR2);
    gpio_set_intr_pullup(BUTTON_PIN_5);
    gpio_set_intr_pullup(BUTTON_PIN_15);

    Queue_EXTI = xQueueCreate(10, sizeof(int));
    xTaskCreate(buttonEXTI_Task, "button_interrupt", 2048, NULL, 0, &xBUTTON_INTERRUPT);
    xTaskCreate(blinkLED_Task, "blink_led", 1024, NULL, 0, &xBLINK_LED);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN_15, external_interrupt_handler, (void*)BUTTON_PIN_15);
    gpio_isr_handler_add(BUTTON_PIN_5, external_interrupt_handler, (void*)BUTTON_PIN_5);
}
