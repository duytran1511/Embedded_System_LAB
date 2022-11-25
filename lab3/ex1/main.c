#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include <inttypes.h>
#include <stdio.h>

void print_task_information(uint8_t id)
{
    printf("Tick %04lu ID %lu\n", xTaskGetTickCount(), id);
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
    while (1)
    {
        printf("Event task %lu was preempted\n", (uint32_t) id);
        print_task_information(3);
        printf("Event task %lu was blocked\n", (uint32_t) id);

        vTaskDelay(30);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    xTaskCreate(continuous_processing_task, "Continous task #1", 2048,
        (void*) 1u, 1, NULL);
    xTaskCreate(continuous_processing_task, "Continous task #2", 2048,
        (void*) 2u, 1, NULL);
    xTaskCreate(higher_priority_task, "Event task #3", 2048, (void*) 3, 2, NULL);

    vTaskDelete(NULL);
}