#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include <inttypes.h>
#include <stdio.h>

#include "sys/time.h"

#define TIMER_1_MESSAGE "ahihi"
#define TIMER_2_MESSAGE "ihaha"

#define TIMER_1_READING_TIME 10
#define TIMER_2_READING_TIME 5

#define TIMER_1_PERIOD 2000
#define TIMER_2_PERIOD 3000

TimerHandle_t timer1;
TimerHandle_t timer2;

BaseType_t xTimer1Started;
BaseType_t xTimer2Started;

uint8_t timer1_reading_time = 0;
uint8_t timer2_reading_time = 0;

struct timeval tv_now;
static inline void print_current_time()
{
    gettimeofday(&tv_now, NULL);
    printf("t = %d\t", tv_now.tv_sec);
}

static void vTimerCallback(TimerHandle_t tim )
{
    uint32_t id = (uint32_t) pvTimerGetTimerID(tim);

    if (id == 0)
    {
        timer1_reading_time++;

        print_current_time();
        printf("%s\t(%d)\n", TIMER_1_MESSAGE, timer1_reading_time);

        if (timer1_reading_time == TIMER_1_READING_TIME)
        {
            if (xTimerStop(tim, 0) == pdFAIL)
            {
                printf("Cound not to stop timer 1\n");
            }
        }
    }
    else if (id == 1)
    {
        timer2_reading_time++;

        print_current_time();
        printf("%s\t(%d)\n", TIMER_2_MESSAGE, timer2_reading_time);

        if (timer2_reading_time == TIMER_2_READING_TIME)
        {
            if (xTimerStop(tim, 0) == pdFAIL)
            {
                printf("Cound not to stop timer 2\n");
            }
        }
    }
    else
    {
        printf("Error: Invalid ID %d\n", id);
    }
}

void app_main(void)
{
    timer1_reading_time = 0;
    timer1 = xTimerCreate(
        "Timer 1",
        pdMS_TO_TICKS(TIMER_1_PERIOD),
        pdTRUE, // Auto reload
        0,
        vTimerCallback);

    timer2_reading_time = 0;
    timer2 = xTimerCreate(
        "Timer 2",
        pdMS_TO_TICKS(TIMER_2_PERIOD),
        pdTRUE, // Auto reload
        1,
        vTimerCallback);

    if ((timer1 != NULL) && (timer2 != NULL))
    {
        xTimer1Started = xTimerStart(timer1, 0);
        xTimer2Started = xTimerStart(timer2, 0);
        // if ((xTimer1Started == pdPASS) && (xTimer2Started == pdPASS))
        // {
        //     ;
        // }
    }
}