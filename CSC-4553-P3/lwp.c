#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

#include "lwp.h"
#include "rr.h"
#include "stack.h"
#include "fp.h"

// The currently executing LWP
static thread current_thread = NULL;

// The original system thread (main program)
static thread main_thread = NULL;

// Next available thread ID
// Thread IDs must be unique and nonzero
static tid_t next_tid = 1;

// Active scheduling policy
// Initially, no scheduler has been explicitly selected
static scheduler current_scheduler = NULL;

// Head of the library's global thread list
static thread all_threads = NULL;

// FIFO queue of terminated threads
static thread terminated_head = NULL; // Oldest terminated thread
static thread terminated_tail = NULL; // Newest terminated thread


static void ensure_scheduler(void) {
    if (current_scheduler == NULL) {
        current_scheduler = RoundRobin;

        if (current_scheduler->init != NULL) {
            current_scheduler->init();
        }
    }
}

// Call to abort if thread wrapper unexpectedly returns
static void lwp_fallback(void) {
    abort();
}

// Entry point for every new LWP
// Run the user's function, then terminate the LWP
// with the returned value
static void lwp_wrap(lwpfun fun, void *arg) {
    int rval = fun(arg);
    lwp_exit(rval);
    // lwp_exit() should never return;
    abort();
}

// Create a new LWP
tid_t lwp_create(lwpfun function, void *arg) {
    thread new_thread;
    unsigned long *stack;
    unsigned long *frame;
    size_t stack_size;

    // Validate function pointer
    if (function == NULL) {
        return NO_THREAD;
    }

    // Allocate and zero initialize the context
    new_thread = calloc(1, sizeof(context));
    if (new_thread == NULL) {
        return NO_THREAD;
    }

    // Allocate the LWP's private stack
    stack = allocate_stack(&stack_size);
    if (stack == NULL) {
        free(new_thread);
        return NO_THREAD;
    }

    // Initialize thread metadata
    new_thread->tid = next_tid++;
    new_thread->stack = stack;
    new_thread->stacksize = stack_size;

    new_thread->state.fxsave = FPU_INIT;
    new_thread->status = MKTERMSTAT(LWP_LIVE, 0);

    // Construct stack frame
    frame = (unsigned long *) ((char *) stack + stack_size);
    frame -= 3;
    frame[0] = 0; // saved rbp
    frame[1] = (unsigned long) (uintptr_t) lwp_wrap; // return address into lwp_wrap
    frame[2] = (unsigned long) (uintptr_t) lwp_fallback; // fallback if lwp_warp returns

    // swap_rfiles() restores rbp and rsp
    // then executes leave; ret
    new_thread->state.rbp = (unsigned long) (uintptr_t) frame;
    new_thread->state.rsp = (unsigned long) (uintptr_t) frame;

    // 1st 2 function arguments under
    // x86-64 System V calling convention
    new_thread->state.rdi = (unsigned long) (uintptr_t) function;
    new_thread->state.rsi = (unsigned long) (uintptr_t) arg;

    // Add context to library's global list of valid threads
    // libe_one is next pointer
    new_thread->lib_one = all_threads;
    all_threads = new_thread;

    // Ensure a scheduler exists and admit new LWP
    ensure_scheduler();
    current_scheduler->admit(new_thread);

    return new_thread->tid;
}