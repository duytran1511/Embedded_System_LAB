#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include <inttypes.h>
#include <stdio.h>


#define GPIO_BUTTON 0

static TaskHandle_t contTask01_handle = NULL;
static TaskHandle_t contTask02_handle = NULL;
static TaskHandle_t eventTask03_handle = NULL;

void IRAM_ATTR button_press_handler(void* arg)
{
    xTaskResumeFromISR(eventTask03_handle);
}

void init_gpio_and_interrupt(void)
{
    gpio_reset_pin(GPIO_BUTTON);
    gpio_set_direction(GPIO_BUTTON, GPIO_MODE_INPUT);

    gpio_set_pull_mode(GPIO_BUTTON, GPIO_PULLUP_ONLY);
    gpio_pullup_en(GPIO_BUTTON);
    
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    gpio_set_intr_type(GPIO_BUTTON, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_BUTTON, button_press_handler, NULL); 
    gpio_intr_enable(GPIO_BUTTON);
}

void print_task_information(uint8_t id)
{
    printf("Tick: %04lu, ID: %lu\n", xTaskGetTickCount(), id);
}

void continuous_processing_task(void *id)
{
    while (1)
    {
        print_task_information((uint32_t) id);
    }

    vTaskDelete(NULL);
}

void higher_priority_task(void *id)
{
    vTaskSuspend(NULL);

    while (1)
    {
        printf("Event task %lu was preempted\n", (uint32_t) id);

        print_task_information(3);

        // go into suspend, the button interrupt will wake this up again
        printf("Event task %lu was suspended\n", (uint32_t) id);

        vTaskSuspend(NULL);
    }
    vTaskDelete(NULL);
}

void app_main()
{
    vTaskPrioritySet(NULL, 10);

    // set up gpio for button and interrupt
    init_gpio_and_interrupt();

    // two continuous task, equal priority
    xTaskCreate(continuous_processing_task, "Continous task #1", 2048,
        (void*) 1u, 1, &contTask01_handle);
    xTaskCreate(continuous_processing_task, "Continous task #2", 2048,
        (void*) 2u, 1, &contTask02_handle);

    // one event task
    xTaskCreate(higher_priority_task, "Event task #3", 2048, (void*) 3, 2, &eventTask03_handle);

    vTaskDelete(NULL);
}