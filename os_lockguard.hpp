/**
 * @file	os_lockguard.hpp
 * @brief	Declaration of FreeRTOS lockguard.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef OS_LOCKGUARD_HPP
#define OS_LOCKGUARD_HPP

#include "os.h"

namespace jungles {

//! Implements RAII for semaphore/mutex locking.
class os_lockguard
{
  public:
    //! The mutex must be created before this constructor is used.
    os_lockguard(os_mutex_t m) noexcept;
    os_lockguard(const os_lockguard &) = delete;
    os_lockguard &operator=(const os_lockguard &) = delete;
    os_lockguard(os_lockguard &&) = delete;
    os_lockguard &operator=(os_lockguard &&) = delete;
    ~os_lockguard();

  private:
    os_mutex_t mux;
};

os_lockguard::os_lockguard(os_mutex_t m) noexcept : mux(m)
{
    os_mutex_take(mux, os_no_timeout);
}
os_lockguard::~os_lockguard()
{
    os_mutex_give(mux);
}

} // namespace jungles

#endif /* OS_LOCKGUARD_HPP */
