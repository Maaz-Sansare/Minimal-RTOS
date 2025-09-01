#include "kernel.h"
#include "port.h"
#include "os.h"
#include "stm32f4xx.h"  // for SysTick and NVIC definitions

#define IDLE_TASK_PRIO 255 // The lowest possible priority

#ifndef MAX_TASKS_COUNT
#   define KERNEL_MAX_TASKS 5
#else
#   define KERNEL_MAX_TASKS MAX_TASKS_COUNT
#endif

static tcb_t __tasks[KERNEL_MAX_TASKS];
static uint8_t __taskCount = 0;
static uint8_t __currentTask = 0;
static uint32_t __sys_tick_count = 0;

static uint32_t idle_stack[128];

void kernel_init(){
    os_create_task(idle_task, NULL, IDLE_TASK_PRIO, idle_stack, 128);
}

/*-------------------- Kernel Internal API --------------------*/
tcb_t* _os_get_task_ptr(uint8_t id) {
    if(id >= __taskCount) return NULL;
    return &__tasks[id];
}

void _os_task_init(tcb_t *tcb, void (*task)(void *), void *arg, uint8_t priority, uint32_t *stack, size_t stack_size) {
    uint32_t *sp = stack + stack_size;
    sp = port_stack_init(task, arg, sp);

    tcb->sp = sp;
    tcb->priority = priority;
    tcb->state = TASK_READY;
    tcb->delay_ticks = 0;
}

tcb_t* _os_scheduler_pick_next(void) {
    uint8_t next_task = __currentTask;
    // Iterate through all tasks to find the next ready one (round-robin)
    for (uint8_t i = 1; i <= __taskCount; i++) {
        next_task = (__currentTask + i) % __taskCount;
        // The idle task (task 0) is always considered ready.
        if (__tasks[next_task].state == TASK_READY || next_task == 0) {
            __currentTask = next_task;
            return &__tasks[__currentTask];
        }
    }
    // This part should theoretically not be reached if the idle task is present.
    return &__tasks[__currentTask];
}

void _os_trigger_context_switch(void) {
    port_yield();
}

void _os_tick_handler(void) {
    __sys_tick_count++;

    for(uint8_t i = 1; i < __taskCount; i++) { // Start from 1 to skip idle task
        if(__tasks[i].state == TASK_BLOCKED) {
            if (_os_get_tick() >= __tasks[i].delay_ticks) {
                __tasks[i].delay_ticks = 0;
                __tasks[i].state = TASK_READY;
            }
        }
    }

    // Always trigger a context switch on every tick.
    // This enables preemption (time-slicing).
    _os_trigger_context_switch();
}

void _os_task_block(tcb_t *tcb, uint32_t ticks) {
    if (ticks <= 0) return;
    tcb->delay_ticks = _os_get_tick() + ticks;
    tcb->state = TASK_BLOCKED;
}

/*-------------------- Application Layer Hooks --------------------*/
int _os_create_task(void (*task)(void *), void *arg, uint8_t priority, uint32_t *stack, size_t stack_size) {
    if(__taskCount >= KERNEL_MAX_TASKS) return -1;
    _os_task_init(&__tasks[__taskCount], task, arg, priority, stack, stack_size);
    return __taskCount++;
}

uint32_t _os_get_tick(void) {
    return __sys_tick_count;
}

uint8_t _os_get_current_task_index(void) {
    return __currentTask;
}