#include "os.h"
#include "kernel.h"
#include "port.h"

void os_init(void) {
    // Kernel internal variables already initialized in static storage
    kernel_init();
}

int os_create_task(void (*task)(void *arg),
                   void *arg,
                   uint8_t priority,
                   uint32_t *stack,
                   size_t stack_size) {
    return _os_create_task(task, arg, priority, stack, stack_size);
}

void os_start(void) {
    port_start_first_task();
    //_os_trigger_context_switch();
}

void os_delay(uint32_t ms) {
    uint8_t idx = _os_get_current_task_index();
    tcb_t *tcb = _os_get_task_ptr(idx);
    if(tcb != NULL) {
        _os_task_block(tcb, ms);
        _os_trigger_context_switch();
    }
}

void os_yield(void) {
    _os_trigger_context_switch();
}

uint32_t os_get_tick(void) {
    return _os_get_tick();
}
