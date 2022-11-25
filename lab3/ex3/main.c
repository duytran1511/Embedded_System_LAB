#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include <inttypes.h>
#include <stdio.h>

int time_count[3] = {0,0,0};

void print_task_information(uint8_t id)
{
    printf("id %u - %5d %5d %5d\n", id, time_count[0],time_count[1],time_count[2]);
}

void task(void *id)
{
    uint8_t task_id = (uint8_t) id;

    while (1)
    {
        time_count[task_id-1] = 0;

        for (int i = 0; i < (40000 - 10000*task_id); i++)
        {
            time_count[task_id-1]++;         
        }
        
        print_task_information(task_id);

        time_count[task_id - 1] = 0;

        taskYIELD();
                
        vTaskDelay(1);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    vTaskPrioritySet(NULL, 10);

    xTaskCreate(task, "id1", 2048,(void*) 1u, 1, NULL);
    xTaskCreate(task, "id2", 2048,(void*) 2u, 2, NULL);
    xTaskCreate(task, "id3", 2048, (void*) 3, 3, NULL);

    vTaskDelete(NULL);
}