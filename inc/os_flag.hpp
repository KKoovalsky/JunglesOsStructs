/**
 * @file	os_flag.hpp
 * @brief	Implements one-to-many RTOS flag.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef OS_FLAG_HPP
#define OS_FLAG_HPP

#include "os.h"

namespace jungles {

/**
 * \brief Implements single-setter-multiple-awaiters RTOS flag.
 *
 * This flag will put a task to the blocked state when it is being awaited. It is thread-safe. Only one task is
 * allowed to call set() method.
 * The flag is reset by default.
 */
class os_flag
{
  public:
    os_flag();
    ~os_flag();

    void wait_set();
    void set();
    void reset();
    bool is_set();

    os_flag(const os_flag &) = delete;
    os_flag(os_flag &&) = delete;
    os_flag &operator=(const os_flag &) = delete;
    os_flag &operator=(os_flag &&) = delete;

  private:
    os_event_group_handle_t event_group;
};

os_flag::os_flag()
{
    event_group = os_event_group_create();
}

os_flag::~os_flag()
{
    os_event_group_delete(event_group);
}

void os_flag::wait_set()
{
    os_event_group_wait_bits_endlessly(event_group, 0x01, os_false, os_false);
}

void os_flag::set()
{
    os_event_group_set_bits(event_group, 0x01);
}

void os_flag::reset()
{
    os_event_group_clear_bits(event_group, 0x01);
}

bool os_flag::is_set()
{
    return os_event_group_get_bits(event_group) & 0x01;
}

} // namespace jungles

#endif /* OS_FLAG_HPP */
