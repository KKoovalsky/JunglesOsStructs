/**
 * @file	os_task.cpp
 * @brief	Definition of FreeRTOS task wrapper.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include "os_task.hpp"

os_task::~os_task() { vTaskDelete(task_handle); }
