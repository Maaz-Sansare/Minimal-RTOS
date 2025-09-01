#ifndef __INCLUDE_MINIMAL_RTOS_PORT_H__
#define __INCLUDE_MINIMAL_RTOS_PORT_H__

#include <stdint.h>

/**
 * @brief idel task.
 * @note When every task is block the code will jump to this.
 */
void idle_task();

/**
 * @brief Start the first task.
 * @note Implemented in assembly. Never returns.
 */
void port_start_first_task(void);

/**
 * @brief Trigger a context switch via PendSV.
 */
void port_yield(void);

/**
 * @brief Initialize system tick timer.
 *
 * @param tick_hz Tick frequency in Hertz.
 */
void port_sys_tick_init(uint32_t tick_hz);

/**
 * @brief Enter a critical section (disable interrupts).
 */
void port_enter_critical(void);

/**
 * @brief Exit a critical section (restore interrupts).
 */
void port_exit_critical(void);

/**
 * @brief Initialize a task stack for first run.
 *
 * @param task Task entry function.
 * @param arg Task argument.
 * @param stack_top Pointer to top of allocated stack.
 * @return Updated stack pointer ready for context restore.
 */
uint32_t* port_stack_init(void (*task)(void *),
                          void *arg,
                          uint32_t *stack_top);

#endif //__INCLUDE_MINIMAL_RTOS_PORT_H__
