/**
 * @file	os_task.hpp
 * @brief	Definition of a wrapper for FreeRTOS task.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef OS_TASK_HPP
#define OS_TASK_HPP

#include "os.h"
#include <functional>

namespace jungles {
/**
 * \brief Wrapper for os_task_create() that allows to work with C++ objects natively.
 *
 * This wrapper allows to pass any functional object to os_task_create() but the final signature must be
 * void(void). This means that any arguments passed to the functional object must be bind at the moment of creating an
 * instance of this class.
 */
class os_task
{
  public:
    /**
     *	\brief The constructor of os_task.
     *	\param[in] task_code		The code of the task.
     *	\param[in] name				The name of the task.
     *	\param[in] stack_size		The size of the stack allocated for the task.
     *	\param[in] priority			The priority of the task.
     */
    template <typename TaskFuncType>
    os_task(TaskFuncType &&task_code, std::string name, unsigned short stack_size, os_base_type_t priority);

    os_task(const os_task &) = delete;
    os_task &operator=(const os_task &) = delete;
    os_task(os_task &&) = delete;
    os_task &operator=(os_task &&) = delete;
    ~os_task();

  private:
    //! The task handle must be stored to delete the task on destruction.
    os_task_handle_t task_handle;

    //! The functional object which stores the task code.
    std::function<void(void)> task_code;
};

template <typename TaskFuncType>
os_task::os_task(TaskFuncType &&task_code, std::string name, unsigned short stack_size, os_base_type_t priority)
    : task_code(std::forward<TaskFuncType>(task_code))
{
    os_task_create(
        [](void *p) {
            // The parameter passed to the task code is a pointer to this object.
            auto os_task_p = static_cast<os_task *>(p);
            os_task_p->task_code();
            os_delay_ms(os_no_timeout);
        },
        name.c_str(),
        stack_size,
        // The parameter of os_task_create() which allows to pass a parameter to the task code is used to pass this
        // object to have acces the task code when the task is executed.
        this,
        priority,
        &task_handle);
}

os_task::~os_task()
{
    os_task_delete(task_handle);
}

} // namespace jungles

#endif /* OS_TASK_HPP */
