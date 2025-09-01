#include "port.h"
#include "kernel.h"
#include "stm32f4xx.h"  // for SysTick and NVIC definitions

void idle_task()
{
    while(1)
    {
        // This is now effectively an idle state, the WFI is good for power saving.
        __WFI();
    }
}

uint32_t* port_stack_init(void (*task)(void*), void* arg, uint32_t* top_of_stack)
{
    uint32_t* sp = top_of_stack;

    // ARM Cortex-M exception stack frame: push initial xPSR, PC, LR, R12, R3-R0
    *(--sp) = 0x01000000;           // xPSR (Thumb bit set)
    *(--sp) = (uint32_t)task;       // PC → task entry
    // NOTE: We don't need a specific LR value here because the context switcher will save/restore it.
    *(--sp) = 0;                    // LR
    *(--sp) = 0x00000012;           // R12
    *(--sp) = 0x00000003;           // R3
    *(--sp) = 0x00000002;           // R2
    *(--sp) = 0x00000001;           // R1
    *(--sp) = (uint32_t)arg;        // R0 → argument

    // For the robust context switcher, we also pre-stack LR and R4-R11
    *(--sp) = 0xFFFFFFFD;           // LR (EXC_RETURN to thread mode, using PSP)

    // Initialize callee-saved registers R4-R11 to 0
    for (int i = 0; i < 8; i++)
        *(--sp) = 0;

    return sp;
}

/*
 * A robust, standard-compliant way to start the first task.
 * This function sets up the PSP and then uses the exception return
 * mechanism to have the CPU correctly restore the task's context.
 */
__attribute__((naked)) void port_start_first_task(void)
{
    __asm volatile(
        "LDR R0, =_os_get_current_task_index\n"
        "BLX R0\n"
        "LDR R1, =_os_get_task_ptr\n"
        "BLX R1\n"                              // Get TCB pointer in R0
        "LDR R0, [R0]\n"                        // Load SP from TCB
        "MSR PSP, R0\n"                         // Set PSP to the task's stack pointer
        "MOV R0, #2\n"                          // Switch to use PSP for thread mode
        "MSR CONTROL, R0\n"
        "ISB\n"                                 // Instruction barrier
        "POP {R4-R11, LR}\n"                    // Pop the stacked R4-R11 and LR
        "POP {R0-R3, R12}\n"                    // Pop the hardware-stacked registers
        "ADD SP, SP, #4\n"                      // Discard the stacked PC
        "POP {PC}\n"                            // Pop the real PC
        ::: "memory"
    );
}

/*
 * A robust context switcher that saves and restores LR along with
 * other callee-saved registers. This is safer and standard practice.
 */
__attribute__((naked)) void PendSV_Handler(void)
{
    __asm volatile(
        "CPSID I\n"                       // Disable interrupts
        "MRS R0, PSP\n"                   // Get current task PSP
        "CBZ R0, PendSV_Handler_NoSave\n" // Skip save if no task has run yet

        // Save the context of the current task
        "STMDB R0!, {R4-R11, LR}\n"      // Save callee-saved registers AND LR

        // Save the current task's updated stack pointer
        "PUSH {R0, LR}\n"
        "BL _os_get_current_task_index\n"
        "BL _os_get_task_ptr\n"
        "POP {R1, LR}\n"
        "STR R1, [R0]\n"                  // Save new SP to the TCB

        "PendSV_Handler_NoSave:\n"
        "PUSH {LR}\n"
        "BL _os_scheduler_pick_next\n"    // Pick the next task to run
        "POP {LR}\n"

        // Restore the context of the next task
        "LDR R0, [R0]\n"                  // Load next task's SP from its TCB
        "LDMIA R0!, {R4-R11, LR}\n"      // Restore callee-saved registers AND LR
        "MSR PSP, R0\n"                   // Update PSP
        "CPSIE I\n"                       // Re-enable interrupts

        "BX LR\n"                         // Return from exception
        ::: "memory"
    );
}


/*-----------------------------------------
  SysTick Handler (1ms tick)
------------------------------------------*/
void SysTick_Handler(void)
{
    _os_tick_handler();
}

/*-----------------------------------------
  Configure SysTick for RTOS
------------------------------------------*/
void port_sys_tick_init(uint32_t tick_hz)
{
    // SystemCoreClock must be defined in STM32Cube HAL
    SysTick_Config(SystemCoreClock / tick_hz);

    // Set PendSV to lowest priority for context switching
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
    // Set SysTick to slightly higher priority than PendSV
    NVIC_SetPriority(SysTick_IRQn, 0xFE);
}

/*-----------------------------------------
  Trigger PendSV to request context switch
------------------------------------------*/
void port_yield(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // Set PendSV pending
    __DSB();                              // Data Synchronization Barrier
    __ISB();                              // Instruction Synchronization Barrier
}
