#ifndef __INCLUDE_MINIMAL_RTOS_KERNEL_H__
#define __INCLUDE_MINIMAL_RTOS_KERNEL_H__

#include <stdint.h>
#include <stddef.h>

/** Task states */
typedef enum {
    TASK_READY,
    TASK_BLOCKED,
    TASK_SUSPENDED
} task_state_t;

/** Task Control Block (TCB) */
typedef struct {
    uint32_t *sp;            /**< Saved stack pointer */
    uint8_t priority;        /**< Task priority */
    task_state_t state;      /**< Current task state */
    uint32_t delay_ticks;    /**< Remaining delay ticks */
    uint32_t mcu_tick_start;    /**< Remaining delay ticks */
} tcb_t;


tcb_t* _os_get_task_ptr(uint8_t id);

/**
 * @brief Initialize the kernel by cretaing a ideal task.
 */
void kernel_init();

/**
 * @brief Initialize a TCB and task stack.
 */
void _os_task_init(tcb_t *tcb, void (*task)(void *), void *arg, uint8_t priority, uint32_t *stack, size_t stack_size);

/**
 * @brief Select next task to run.
 *
 * @return Pointer to next TCB.
 */
tcb_t* _os_scheduler_pick_next(void);

/**
 * @brief Tick handler called from SysTick ISR.
 */
void _os_tick_handler(void);

/**
 * @brief Request a context switch.
 */
void _os_trigger_context_switch(void);

/**
 * @brief Block a task for a given tick count.
 */
void _os_task_block(tcb_t *tcb, uint32_t ticks);

/**
 * @brief Mark task as ready to run.
 */
void _os_task_ready(tcb_t *tcb);

int _os_create_task(void (*task)(void *), void *arg, uint8_t priority, uint32_t *stack, size_t stack_size);

tcb_t* _os_get_task_ptr(uint8_t id);
uint8_t _os_get_current_task_index(void);
void _os_set_current_task_index(uint8_t idx);
uint32_t _os_get_tick(void);


#endif /* __INCLUDE_MINIMAL_RTOS_KERNEL_H__ */
