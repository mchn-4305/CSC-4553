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
    }
    if (current_scheduler->init != NULL) {
        current_scheduler->init();
    }
}

tid_t lwp_create(lwpfun function, void *arg) {
    thread new_thread;
    unsigned long *stack;
    size_t stack_size;
    size_t num_pages;

    if (function == NULL) {
        return NO_THREAD;
    }

    new_thread = malloc(sizeof(context));
    if (new_thread == NULL) {
        return NO_THREAD;
    }

    stack = allocate_stack(&stack_size);
    if (stack == NULL) {
        free(new_thread);
        return NO_THREAD;
    }
    new_thread->tid = next_tid++;
    new_thread->stack = stack;
    new_thread->stacksize = stack_size;
    new_thread->state.fxsave = FPU_INIT;
    new_thread->status = MKTERMSTAT(LWP_LIVE, 0);
    new_thread->lib_one = NULL;
    new_thread->lib_two = NULL;
    new_thread->sched_one = NULL;
    new_thread->sched_two = NULL;
    new_thread->exited = NULL;

    // INCOMPLETE



    return new_thread->tid;
}