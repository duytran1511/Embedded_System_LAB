#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>

#define GPIO_BUTTON GPIO_NUM_12
#define SHORT_TASK_LOOPS_COUNT 2000000u
#define YIELD_TASK_LOOPS_COUNT 2000000u

static uint32_t taskID = -1u;
static uint32_t taskID_old = -1u;

// Idle hook
void vApplicationIdleHook(void)
{
    taskID = taskID_old = 0u;
}

void print_task_information(void)
{
    if (taskID != taskID_old)
    {
        taskID_old = taskID;
        printf("Tick: %04lu, ID: %lu\n", xTaskGetTickCount(), taskID);
    }
}

static TaskHandle_t yieldTask01_handle = NULL;
static TaskHandle_t yieldTask02_handle = NULL;
static TaskHandle_t yieldTask03_handle = NULL;
static TaskHandle_t yieldTask04_handle = NULL;

void IRAM_ATTR button_press_handler(void* arg)
{
    xTaskResumeFromISR(yieldTask01_handle);
    xTaskResumeFromISR(yieldTask02_handle);
    xTaskResumeFromISR(yieldTask03_handle);
    xTaskResumeFromISR(yieldTask04_handle);
}

void initializeGPIO(void)
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

// these task run for a while (YIELD_TASK_LOOPS_COUNT) and then yield
void vTaskWithYield(void *param)
{
    uint32_t taskID = (uint32_t) param;
    // delay starting the task by taskid * 10 ticks
    vTaskDelay(10 * taskID);

    while (1)
    {
        for (UBaseType_t i = 0; i < YIELD_TASK_LOOPS_COUNT; i++)
        {
            taskID = taskID;
            print_task_information();
        }

        if (taskID > 1)
        {
            // suspend higher priority task to simulate them going into blocked state
            // they will be resume on button press
            vTaskSuspend(NULL);
        }
        else
        {
            taskYIELD();
        }
    }

    vTaskDelete(NULL);
}


void app_main(void)
{
    vTaskPrioritySet(NULL, 10);

    initializeGPIO();

    // four task with yield, increasing priority, id = priority
    xTaskCreate(vTaskWithYield, "Yielding task #1", 2048, (void*) 1u, 1, &yieldTask01_handle);
    xTaskCreate(vTaskWithYield, "Yielding task #2", 2048, (void*) 2u, 2, &yieldTask02_handle);
    xTaskCreate(vTaskWithYield, "Yielding task #3", 2048, (void*) 3u, 3, &yieldTask03_handle);
    xTaskCreate(vTaskWithYield, "Yielding task #4", 2048, (void*) 4u, 4, &yieldTask04_handle);
    
    vTaskDelete(NULL);
}