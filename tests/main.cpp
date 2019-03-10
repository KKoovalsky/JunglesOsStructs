/**
 * @file	main.cpp
 * @brief	Definition of main() for unit tests.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include "FreeRTOS.h"
#include "task.h"
#include "unity.h"

extern void test_os_char_driver();

int main()
{
    UNITY_BEGIN();

    TaskHandle_t test_task_handle;
    xTaskCreate(
        [](void *) {
            test_os_char_driver();

            vTaskEndScheduler();
        },
        "rtos_test",
        2048,
        NULL,
        1,
        NULL/* &test_task_handle */);

    vTaskStartScheduler();

    // vTaskDelete(test_task_handle);

    return UNITY_END();
}
