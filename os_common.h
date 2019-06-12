/**
 * @file	os_common.h
 * @brief	Implements common functions which uses basic RTOS functionalities
 * @author	Kacper Kowalski - kacper.kowalski@lerta.energy
 */

#ifndef OS_COMMON_H
#define OS_COMMON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Wait for the predicate to return true after specific timeout.
 *
 * \param[in] delay_each_check_ms The delay in milliseconds between each call to the predicate.
 * \param[in] timeout_ms The timeout in milliseconds after which the function stops asking the predicate.
 * \param[in] predicate The predicate which is asked to return true within the specified timeout.
 * \param[in] context The pointer to the structure passed to the predicate.
 *
 * \returns true when the predicate returns true within the specified timeout, false otherwise.
 */
bool os_wait(unsigned delay_each_check_ms, unsigned timeout_ms, bool (*predicate)(void *context), void *context);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OS_COMMON_H */
