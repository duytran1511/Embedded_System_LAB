#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include <inttypes.h>
#include <stdio.h>

#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "bootloader_random.h"
#include "esp_random.h"

// struct data for queue
typedef struct
{
    uint8_t ID;
    uint32_t val;
} Data;
// this variable hold queue handle
xQueueHandle xQueue;

// variables and constants for request
/*
    0 - get temperature
    1 - Turn on LED
    2 - Turn off LED
    the other - unacceptable request
*/
#define GET_TEMP        0
#define TURN_ON_LED     1
#define TURN_OFF_LED    2

#define N_REQUEST       3
uint8_t request[N_REQUEST] = {0, 0, 0};
// check if there is any request
bool flagRequest = false;

// create request task
void vRequest();
// reception task
void vReceptionTask(void);
// functional tasks
void vGetTemp(void *parameter);
void vSetLed(void *parameter);

void app_main()
{
    /* create the queue which size can contains 5 elements of Data */
    xQueue = xQueueCreate(5, sizeof(Data));

    xTaskCreate(vRequest, "vRequest", 2048, NULL, 3, NULL);

    xTaskCreate(vReceptionTask, "vReceptionTask1", 2048, NULL, 2, NULL);

    xTaskCreate(vGetTemp, "vGetTemp1", 2048, (uint8_t *)0u, 1, NULL); // task get temp, id = 0
    xTaskCreate(vSetLed, "vSetLed", 2048, (uint8_t *)1u, 1, NULL);    // task set led, id = 1
}

void vRequest()
{
    uint8_t i = 0;

    while (1)
    {
        // create 3 requests every 5 seconds

        // create request randomly from 0 to 3
        // if request == 3 then error
        bootloader_random_enable();
        request[0] = esp_random() % 4;
        request[1] = esp_random() % 4;
        request[2] = esp_random() % 4;
        bootloader_random_disable();

        // set flag to notify there is request
        flagRequest = true;

        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

void vReceptionTask(void)
{
    /* keep the status of sending data */
    BaseType_t xStatus;
    /* time to block the task until the queue has free space */
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    /* Data is added to queue*/
    Data requestedData;

    uint8_t i = 0;

    while (1)
    {
        // check if there is any request
        if (flagRequest)
        {
            // classify 3 tasks,
            // then add data to queue
            for (i = 0; i < N_REQUEST; i++)
            {
                if (request[i] == GET_TEMP)
                {
                    requestedData.ID = 0;
                    requestedData.val = 0;
                    printf("Request: get temperature\n");
                }
                else if (request[i] == TURN_ON_LED)
                {
                    requestedData.ID = 1;
                    requestedData.val = 1;
                    printf("Request: Turn on LED\n");
                }
                else if (request[i] == TURN_OFF_LED)
                {
                    requestedData.ID = 1;
                    requestedData.val = 0;
                    printf("Request: Turn off LED\n");
                }
                else
                {
                    // If no functional task receives the request,
                    // raise an error
                    requestedData.ID = request[i];
                    requestedData.val = 0;
                    printf("Request %d: Error\n", request[i]);

                    // unset flag, and ignore that request
                    flagRequest = false;
                    continue;
                }

                // send data to queue
                xStatus = xQueueSendToFront(xQueue, &requestedData, xTicksToWait);

                // check if sending is ok or not
                if (xStatus == pdPASS)
                {
                    ;
                }
                else
                {
                    printf("Request %d: Could not send data\n", request);
                }
            }

            // unset flag
            flagRequest = false;
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void vGetTemp(void *parameter)
{
    // keep the status of receiving data
    BaseType_t xStatus;
    // time to block the task until data is available
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

    Data data;
    uint8_t id = (uint8_t)parameter;

    uint32_t temperature = 0;

    while (1)
    {
        // Peek data from the queue
        // to check if the next request is for it
        xStatus = xQueuePeek(xQueue, &data, xTicksToWait);

        if (xStatus == pdPASS)
        {
            // check if request
            if (data.ID == id)
            {
                // get random temperature
                bootloader_random_enable();
                temperature = 25 + (esp_random() % 5);
                bootloader_random_disable();

                // pop data from the queue
                xStatus = xQueueReceive(xQueue, &data, xTicksToWait);

                if (xStatus == pdPASS)
                {
                    printf("Response: %d*C\n", temperature);
                }
                else
                {
                    printf("Response: Cound not recieve data\n");
                }
            }
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void vSetLed(void *parameter)
{
    // keep the status of receiving data
    BaseType_t xStatus;
    // time to block the task until data is available
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

    Data data;
    uint8_t id = (uint8_t)parameter;

    while (1)
    {
        // Peek data from the queue
        // to check if the next request is for it
        xStatus = xQueuePeek(xQueue, &data, xTicksToWait);

        if (xStatus == pdPASS)
        {
            // check if request
            if (data.ID == id)
            {
                xStatus = xQueueReceive(xQueue, &data, xTicksToWait);

                // pop data from the queue
                if (xStatus == pdPASS)
                {
                    printf("Response: Set LED to %d\n", data.val);
                }
                else
                {
                    printf("Response: Cound not recieve data\n");
                }
            }
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}