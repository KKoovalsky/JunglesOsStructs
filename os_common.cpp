/**
 * @file	os_common.c
 * @brief	Implements the common functions which use basic RTOS functionalities
 * @author	Kacper Kowalski - kacper.kowalski@lerta.energy
 */
#include "os.h"

// --------------------------------------------------------------------------------------------------------------------
// DEFINITIONS OF STRUCTURES, DATA TYPES, ...
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// DECLARATION OF PRIVATE FUNCTIONS AND VARIABLES
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// DEFINITION OF PUBLIC FUNCTIONS AND VARIABLES
// --------------------------------------------------------------------------------------------------------------------
extern "C" bool
os_wait(unsigned delay_each_check_ms, unsigned timeout_ms, bool (*predicate)(void *context), void *context)
{
    const unsigned num_iters = timeout_ms / delay_each_check_ms;
    for (unsigned i = 0; i < num_iters; ++i)
    {
        os_delay_ms(delay_each_check_ms);
        if (predicate(context))
            return true;
    }
    return false;
}

// --------------------------------------------------------------------------------------------------------------------
// DEFINITION OF PRIVATE FUNCTIONS
// --------------------------------------------------------------------------------------------------------------------

