# Minimal Portable RTOS

A lightweight, preemptive, priority-based real-time operating system for ARM Cortex-M microcontrollers.

## Introduction

This project is a minimal, portable real-time operating system (RTOS) designed for embedded systems, particularly those based on the ARM Cortex-M architecture. The RTOS is built with a clean, layered architecture to simplify portability to new microcontrollers.

The system is structured in three distinct layers:
*   **Application Layer:** A simple, public API for creating tasks and managing time.
*   **Kernel Core:** An MCU-agnostic core that handles task management, the scheduling policy, and system ticks.
*   **Port Layer:** The hardware-specific layer that handles the low-level context switching, stack management, and timer configuration.

The core design principle is portability: to run this RTOS on a new microcontroller, only the port layer needs to be implemented.

## Features

*   **Priority-Based Preemptive Scheduler:** Ensures that the highest-priority task that is ready to run is always the one executing.
*   **Round-Robin for Equal Priority:** Tasks at the same priority level share CPU time in a round-robin fashion.
*   **Preemptive Multitasking:** The SysTick timer interrupt drives the scheduler, allowing higher-priority tasks to preempt lower-priority ones the moment they become ready.
*   **Task Management:** Dynamically create tasks with their own stack, arguments, and priority.
*   **Time Management:** Support for blocking task delays (`os_delay`).
*   **Idle Task:** Includes a default idle task with the lowest priority that runs when no other tasks are ready, enabling the use of power-saving modes like `__WFI()` (Wait For Interrupt).
*   **Modular and Portable:** The kernel is completely hardware-independent, and all MCU-specific code is isolated in the port layer.

## Layered Architecture

The RTOS is designed in three layers, which separates the application logic from the hardware.

#### Application Layer (`os.h`)
This is the user-facing API. It provides the functions needed to interact with the RTOS.
*   `os_init()`
*   `os_create_task()`
*   `os_start()`
*   `os_delay()`

#### Kernel Core (`kernel.c`, `kernel.h`)
This is the heart of the RTOS. It is completely platform-agnostic.
*   Manages Task Control Blocks (TCBs).
*   Implements the priority-based scheduling algorithm.
*   Handles the system tick and manages task delays.

#### Port Layer (`port.c`, `port.h`)
This layer makes the RTOS work on specific hardware. It is the only part that needs to be changed when porting to a new microcontroller.
*   Implements the low-level context switch (`PendSV_Handler`).
*   Handles the initial stack frame setup for new tasks (`port_stack_init`).
*   Configures the system timer (`SysTick`).

## Getting Started

Follow these steps to integrate the RTOS into your project.

1.  Include the `os.h`, `kernel.h`, and `port.h` files in your project.
2.  Ensure the port layer is implemented for your target MCU (an implementation for STM32F4 is provided).
3.  In your `main` function, initialize the RTOS using `os_init()`. This will also create the idle task.
4.  Create your application tasks using `os_create_task()`. Remember that a **lower number indicates a higher priority**.
5.  Start the scheduler with `os_start()`. This function will not return.

### Example Usage:

```c
#include "os.h"

// Stacks for the tasks
static uint32_t task1_stack;
static uint32_t task2_stack;

// Task functions
void high_prio_task(void* arg) {
    while(1) {
        // This task will run frequently
        os_delay(100);
    }
}

void low_prio_task(void* arg) {
    while(1) {
        // This task will only run when high_prio_task is blocked
        os_delay(500);
    }
}

int main(void) {
    // Hardware initialization (clocks, GPIOs, etc.)

    // Initialize the RTOS (creates the idle task at lowest priority)
    os_init();

    // Create application tasks (lower number = higher priority)
    os_create_task(high_prio_task, NULL, 1, task1_stack, 128); // Priority 1 (High)
    os_create_task(low_prio_task,  NULL, 2, task2_stack, 128); // Priority 2 (Low)

    // Start the scheduler
    os_start(); // This call never returns

    // The idle task will run if both tasks above are in an os_delay
    while(1);
}