/**
 * @file	os_flag.cpp
 * @brief	Implements one-to-many RTOS flag.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include "os_flag.hpp"

os_flag::os_flag() { event_group = xEventGroupCreate(); }

os_flag::~os_flag() { vEventGroupDelete(event_group); }

void os_flag::wait_set() { xEventGroupWaitBits(event_group, 0x01, pdFALSE, pdFALSE, portMAX_DELAY); }

void os_flag::set() { xEventGroupSetBits(event_group, 0x01); }

void os_flag::reset() { xEventGroupClearBits(event_group, 0x01); }

bool os_flag::is_set() { return xEventGroupGetBits(event_group) & 0x01; }
