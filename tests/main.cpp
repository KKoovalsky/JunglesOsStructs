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

    xTaskCreate(
        [](void *) {
            test_os_char_driver();

            vTaskEndScheduler();
        },
        "rtos_test",
        2048,
        NULL,
        1,
        NULL);

    vTaskStartScheduler();

    return UNITY_END();
}
