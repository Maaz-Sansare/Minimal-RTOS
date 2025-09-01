#ifndef __INCLUDE_MINIMAL_RTOS_OS_H__
#define __INCLUDE_MINIMAL_RTOS_OS_H__

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Initialize RTOS kernel structures.
 * @note Must be called before creating tasks.
 */
void os_init(void);

/**
 * @brief Start the RTOS scheduler.
 * @note This function never returns.
 */
void os_start(void);

/**
 * @brief Create a new task.
 *
 * @param task Function pointer to the task entry.
 * @param arg Pointer passed as argument to the task function.
 * @param priority Task priority (0 = lowest).
 * @param stack Pointer to pre-allocated stack buffer.
 * @param stack_size Size of the stack buffer in words.
 * @return Task ID (>=0) on success, negative on error.
 */
int os_create_task(void (*task)(void *arg), void *arg, uint8_t priority, uint32_t *stack, size_t stack_size);

/**
 * @brief Yield CPU voluntarily to another task.
 */
void os_yield(void);

/**
 * @brief Block the current task for a number of milliseconds.
 *
 * @param ms Duration in milliseconds.
 */
void os_delay(uint32_t ms);

/**
 * @brief Get system uptime in ticks.
 *
 * @return Tick count since OS start.
 */
uint32_t os_get_tick(void);

#endif //__INCLUDE_MINIMAL_RTOS_OS_H__
