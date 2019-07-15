/**
 * @file	os.h
 * @brief	Defines OS layer calls which abstract the underlying operating system.
 * @author	Kacper Kowalski - kacper.kowalski@lerta.energy
 */

#ifndef OS_H
#define OS_H

#ifdef __FREERTOS__

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

typedef TaskHandle_t os_task_handle_t;
typedef BaseType_t os_base_type_t;
typedef EventGroupHandle_t os_event_group_handle_t;
typedef EventBits_t os_event_bits_t;
typedef SemaphoreHandle_t os_counting_semaphore_t;
typedef SemaphoreHandle_t os_binary_semaphore_t;
typedef SemaphoreHandle_t os_recursive_mutex_t;
typedef SemaphoreHandle_t os_mutex_t;
typedef TickType_t os_tick_type_t;
typedef TimerHandle_t os_timer_handle_t;

#define os_true pdTRUE
#define os_false pdFALSE
#define os_fail pdFAIL
#define os_pass pdPASS

#define os_no_timeout portMAX_DELAY

#define os_task_state_ready eReady
#define os_task_state_running eRunning
#define os_task_state_blocked eBlocked
#define os_task_state_suspended eSuspended
#define os_task_state_deleted eDeleted

#define os_task_create(code, name, stack_size, params, priority, task_handle_addr)                                     \
    xTaskCreate(code, name, stack_size, params, priority, task_handle_addr)
#define os_task_delete_this() vTaskDelete(NULL)
#define os_task_delete(task_handle) vTaskDelete(task_handle)
#define os_task_get_current_task_handle() xTaskGetCurrentTaskHandle()
#define os_task_yield taskYIELD
#define os_task_yield_from_isr portYIELD_FROM_ISR
#define os_task_get_state(task_handle) eTaskGetState(task_handle)
#define os_wait_endlessly_for_notification() ulTaskNotifyTake(pdTRUE, portMAX_DELAY)
#define os_wait_for_notification_ms(timeout_ms) ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms))
#define os_notify(task_handle) xTaskNotifyGive(task_handle)
#define os_notify_give_from_isr(task_handle, higher_prior_task_woken)                                                  \
    vTaskNotifyGiveFromISR(task_handle, higher_prior_task_woken)
#define os_clear_this_tasks_notification() ulTaskNotifyTake(pdTRUE, 0)

#define os_event_group_create() xEventGroupCreate()
#define os_event_group_delete(event_group) vEventGroupDelete(event_group)
#define os_event_group_set_bits(event_group, bits_to_set) xEventGroupSetBits(event_group, bits_to_set)
#define os_event_group_wait_bits_endlessly(event_group, bits_to_wait_for, clear_on_exit, wait_for_all)                 \
    xEventGroupWaitBits(event_group, bits_to_wait_for, clear_on_exit, wait_for_all, portMAX_DELAY)
#define os_event_group_clear_bits(event_group, bits_to_clear) xEventGroupClearBits(event_group, bits_to_clear)
#define os_event_group_get_bits(event_group) xEventGroupGetBits(event_group)
#define os_counting_semaphore_create(max_count, initial_count) xSemaphoreCreateCounting(max_count, initial_count)
#define os_counting_semaphore_delete(semaphore) vSemaphoreDelete(semaphore)
#define os_counting_semaphore_give_from_isr(semaphore) os_semaphore_give_from_isr(semaphore)
#define os_counting_semaphore_take(semaphore, timeout) xSemaphoreTake(semaphore, os_timeout_to_ticks(timeout))
#define os_recursive_mutex_create() xSemaphoreCreateRecursiveMutex()
#define os_recursive_mutex_delete(mutex) vSemaphoreDelete(mutex)
#define os_recursive_mutex_take(mutex, timeout) xSemaphoreTakeRecursive(mutex, os_timeout_to_ticks(timeout))
#define os_recursive_mutex_give(mutex) xSemaphoreGiveRecursive(mutex)
#define os_binary_semaphore_create() xSemaphoreCreateBinary()
#define os_binary_semaphore_delete(mutex) vSemaphoreDelete(mutex)
#define os_binary_semaphore_take(mutex, timeout) xSemaphoreTake(mutex, os_timeout_to_ticks(timeout))
#define os_binary_semaphore_give(mutex) xSemaphoreGive(mutex)
#define os_binary_semaphore_take_from_isr(mutex, higher_prior_task_woken)                                              \
    xSemaphoreTakeFromISR(mutex, higher_prior_task_woken)
#define os_binary_semaphore_give_from_isr(mutex, higher_prior_task_woken)                                              \
    xSemaphoreGiveFromISR(mutex, higher_prior_task_woken)
#define os_mutex_create() xSemaphoreCreateMutex()
#define os_mutex_delete(mutex) vSemaphoreDelete(mutex)
#define os_mutex_take(mutex, timeout) xSemaphoreTake(mutex, os_timeout_to_ticks(timeout))
#define os_mutex_give(mutex) xSemaphoreGive(mutex)
#define os_timer_create(name, period_ms, do_auto_reload, timer_id, clbk)                                               \
    xTimerCreate(name, os_timeout_to_ticks(period_ms), do_auto_reload, timer_id, clbk)
#define os_timer_get_id(tim) pvTimerGetTimerID(tim)
#define os_timer_stop(tim, timeout_ms) xTimerStop(tim, os_timeout_to_ticks(timeout_ms))
#define os_timer_change_period_and_reset(tim, new_period_ms, timeout_ms)                                               \
    xTimerChangePeriod(tim, os_timeout_to_ticks(new_period_ms), os_timeout_to_ticks(timeout_ms))

#define os_timeout_to_ticks(timeout_ms) (timeout_ms == portMAX_DELAY ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms))

#define os_get_free_heap_size() xPortGetFreeHeapSize()
#define os_get_minimum_ever_free_heap_size() xPortGetMinimumEverFreeHeapSize()

#define os_scheduler_start() vTaskStartScheduler()

static inline void os_notify_from_isr(os_task_handle_t task_handle)
{
    BaseType_t higher_prior_task_woken = pdFALSE;
    vTaskNotifyGiveFromISR(task_handle, &higher_prior_task_woken);
    portEND_SWITCHING_ISR(higher_prior_task_woken);
}

static inline void os_event_group_set_bits_from_isr(os_event_group_handle_t event_group, os_event_bits_t bits_to_set)
{
    BaseType_t higher_prior_task_woken = pdFALSE;
    xEventGroupSetBitsFromISR(event_group, bits_to_set, &higher_prior_task_woken);
    portEND_SWITCHING_ISR(higher_prior_task_woken);
}

static inline void os_semaphore_give_from_isr(os_binary_semaphore_t semaphore)
{
    BaseType_t higher_prior_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(semaphore, &higher_prior_task_woken);
    portEND_SWITCHING_ISR(higher_prior_task_woken);
}

static inline void os_delay_ms(unsigned timeout)
{
    TickType_t t = os_timeout_to_ticks(timeout);
    vTaskDelay(t);
}

#elif defined(__MBED__)

#include "mbed.h"

#define os_delay_ms(timeout) ThisThread::sleep_for(timeout)

#else /* __FREERTOS__ */
#warning "No target OS defined"

typedef unsigned os_task_handle_t;
typedef unsigned os_base_type_t;
typedef unsigned os_event_group_handle_t;
typedef unsigned os_event_bits_t;
typedef unsigned os_counting_semaphore_t;
typedef unsigned os_binary_semaphore_t;
typedef unsigned os_recursive_mutex_t;
typedef unsigned os_mutex_t;
typedef unsigned os_tick_type_t;
typedef unsigned os_timer_handle_t;

#define os_true 1
#define os_false 0
#define os_fail 0
#define os_pass 1

#define os_no_timeout 0

#define os_task_state_ready 0
#define os_task_state_running 0
#define os_task_state_blocked 0
#define os_task_state_suspended 0
#define os_task_state_deleted 0

#define os_task_create(code, name, stack_size, params, priority, task_handle_addr) empty_fun(0)
#define os_task_delete(task_handle) empty_fun(0)
#define os_task_get_current_task_handle() empty_fun(0)
#define os_wait_endlessly_for_notification() empty_fun(0)
#define os_wait_for_notification_ms(timeout_ms) empty_fun(0)
#define os_notify_from_isr(task_handle) empty_fun(0)
#define os_notify_give_from_isr(task_handle) empty_fun(0)
#define os_notify(task_handle) empty_fun(0)
#define os_clear_this_tasks_notification() empty_fun(0)
#define os_delay_ms(delay_ms) empty_fun(0)
#define os_task_delete_this() empty_fun(0)
#define os_task_yield() empty_fun(0)
#define os_task_yield_from_isr() empty_fun(0)
#define os_task_get_state(task_handle) empty_fun(0)
#define os_event_group_create() empty_fun(0)
#define os_event_group_delete(event_group) empty_fun(0)
#define os_event_group_set_bits(event_group, bits_to_set) empty_fun(0)
#define os_event_group_wait_bits_endlessly(event_group, bits_to_wait_for, clear_on_exit, wait_for_all) empty_fun(0)
#define os_event_group_clear_bits(event_group, bits_to_clear) empty_fun(0)
#define os_event_group_set_bits_from_isr(event_group, bits_to_set) empty_fun(0)
#define os_event_group_get_bits(event_group) empty_fun(0)
#define os_get_free_heap_size() empty_fun(0)
#define os_get_minimum_ever_free_heap_size() empty_fun(0)
#define os_recursive_mutex_create() empty_fun(0)
#define os_recursive_mutex_delete(mutex) empty_fun(0)
#define os_recursive_mutex_take(mutex, timeout) empty_fun(0)
#define os_recursive_mutex_give(mutex) empty_fun(0)
#define os_mutex_create() empty_fun(0)
#define os_mutex_delete(mutex) empty_fun(0)
#define os_mutex_take(mutex, timeout_ms) empty_fun(0)
#define os_mutex_give(mutex) empty_fun(0)
#define os_counting_semaphore_create(max_count, initial_count) empty_fun(0)
#define os_counting_semaphore_delete(semaphore) empty_fun(0)
#define os_counting_semaphore_give_from_isr(semaphore) empty_fun(0)
#define os_counting_semaphore_take(semaphore, timeout) empty_fun(0)
#define os_binary_semaphore_create() empty_fun(0)
#define os_binary_semaphore_delete(mutex) empty_fun(0)
#define os_binary_semaphore_take(mutex, timeout) empty_fun(0)
#define os_binary_semaphore_give(mutex) empty_fun(0)
#define os_binary_semaphore_take_from_isr(mutex, higher_prior_task_woken) empty_fun(0)
#define os_binary_semaphore_give_from_isr(mutex, higher_prior_task_woken) empty_fun(0)
#define os_semaphore_give_from_isr(semaphore) empty_fun(0)
#define os_timer_create(name, period_ms, do_auto_reload, timer_id, clbk) empty_fun(0)
#define os_timer_get_id(tim) empty_fun(0)
#define os_timer_stop(tim, timeout_ms) empty_fun(0)
#define os_timer_change_period_and_reset(tim, new_period_ms, timeout_ms) empty_fun(0)
#define os_timeout_to_ticks(timeout) empty_fun(0)
#define os_scheduler_start() empty_fun(0)

static inline unsigned empty_fun(unsigned retval)
{
    return retval;
}

#endif /* __FREERTOS__ */

#endif /* OS_H */
