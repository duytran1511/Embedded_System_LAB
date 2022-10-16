#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#define BUTTON_GPIO 2

#define IDENTIFIER "Group 2:\n1. Tran Nguyen Minh Duy - 1910095\n2. Dang Trung Kien - 1911437\n3. Nguyen Hai Long - 1911517\n4. Nguyen Nhat Truong - 1912344\n"

short key_code = 0;

void init_system()
{
    key_code = 0;

    // button
    gpio_pad_select_gpio(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
}

void monitor_task(void *pvParameter)
{
	while(1)
	{
	    printf(IDENTIFIER);
	    vTaskDelay(1000 / portTICK_RATE_MS);
	}

    vTaskDelete(NULL);
}

void buttton_task(void *pvParameter)
{
    
    while(1) {
        if (gpio_get_level(BUTTON_GPIO) == 1)
        {
            key_code++;            
        }
        else
        {
            key_code = 0;
        }

        vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void is_button_pressed(void *pvParameter)
{
    while(1) {
        if (key_code == 5)
        {
            printf("ESP32\n");
        }

        vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    init_system();

    xTaskCreate(&monitor_task, "monitor_task", 1024, NULL, 1, NULL);
    xTaskCreate(&buttton_task, "buttton_task", 1024, NULL, 2, NULL);
    xTaskCreate(&is_button_pressed, "is_button_pressed", 1024, NULL, 2, NULL);
    vTaskStartScheduler();
}