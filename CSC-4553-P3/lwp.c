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

//FIFO queue of waiting thread
static thread waiting_head = NULL; //Oldest waiting thread
static thread waiting_tail = NULL; // Newest waiting thread


// Helpers for waiting and terminated threads
static void enqueue(thread *head, thread *tail, thread t) {
    t->lib_two = NULL;

    if (*tail != NULL) {
        (*tail)->lib_two = t;
    } else {
        //empty queue
        *head = t;
    }
    
    *tail = t;
}

static thread dequeue(thread *head, thread *tail) {
    thread t = *head;

    if (t != NULL) {
        *head = t->lib_two;
        
        // if queue is now empty
        if (*head == NULL) {
            *tail = NULL;
        }
        t->lib_two = NULL;
    }
    return t;
}

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
    frame[2] = (unsigned long) (uintptr_t) lwp_fallback; // fallback if lwp_wrap returns

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

    void lwp_start(void) {
        thread t;

        // make sure original thread is the only one converted
        if (main_thread != NULL) {
            return;
        }

        t = calloc(1, sizeof(context));
        if (t == NULL) {
            return;
        }
        
        // create context
        t->tid = next_tid++;
        t->stack = NULL;
        t->stacksize = 0;
        t->status = MKTERMSTAT(LWP_LIVE, 0);

        // add to library's list of valid threads
        t->lib_one = all_threads;

        all_threads = t;
        main_thread = t;
        current_thread = t;

        //ensure scheduler exists + admit original thread
        ensure_scheduler();
        current_scheduler->admit(t);
        lwp_yield();
    }

    void lwp_yield(void) {
        thread old_thread;
        thread next_thread;

        // lwp_start() has not run yet
        if (current_thread == NULL) {
            return;
        }
        old_thread = current_thread;
        next_thread = current_scheduler->next();

        // no runnable threads left
        if (next_thread == NULL) {
            exit(LWPTERMSTAT(old_thread->status));
        }
        
        // scheduler returns same thread
        if (next_thread == old_thread) {
            return;
        }

        // context switch old thread and new thread
        current_thread = next_thread;
        swap_rfiles(&old_thread->state, &next_thread->state);
    }

    void lwp_exit(int exitval) {
        thread waiting_thread;

        if (current_thread == NULL) {
            exit(exitval & 0xFF);
        }

        // mark terminated
        current_thread->status = MKTERMSTAT(LWP_TERM, exitval);

        // remove from scheduler
        current_scheduler->remove(current_thread);

        //check for waiting thread
        waiting_thread = dequeue(&waiting_head, &waiting_tail);
        if (waiting_thread != NULL) {
            waiting_thread->exited = current_thread;
            current_scheduler->admit(waiting_thread);
        } else {
            enqueue(&terminated_head, &terminated_tail, current_thread);
        }

        lwp_yield();
        abort();
    }

    tid_t lwp_gettid(void) {
        if (current_thread == NULL) {
            return NO_THREAD;
        }
        return current_thread->tid;
    }

    thread tid2thread(tid_t tid) {
        if (tid == NO_THREAD) {
            return NULL;
        }

        thread current = all_threads;
        while (current != NULL) {
            if (current->tid == tid) {
                return current;
            }
            current = current->lib_one;
        }

        return NULL;
    }

    static void remove_from_all_threads(thread victim) {
        thread *current = &all threads;
        while (*current != NULL) {
            if (*current == victim) {
                *current = victim->lib_one;
                victim->lib_one = NULL;
                return;
            }

            current = &(*current)->lib_one;
        }
    }

    tid_t lwp_wait(int *status) {
        thread finished;

        // No active LWP can wait
        if (current_thread == NULL) {
            return NO_THREAD;
        }

        // A terminated thread is already available
        finished = dequeue(&terminated_head, &terminated_tail);

        if (finished == NULL) {
            // No thread has terminated yet
            // check whether another runnable thread
            // could eventually terminate
            if (current_scheduler->qlen() <= 1) {
                return NO_THREAD;
            }

            // remove the calling thread form the runnable scheduler
            current_scheduler->remove(current_thread);
            // place it at end of waiting queue
            current_thread->exited = NULL;
            enqueue(&waiting_head, &waiting_tail, current_thread);

            // switch to another thread
            lwp_yield();
            finished = current_thread->exited;
            current_thread->exited = NULL;
            if (finished == NULL) {
                return NO_THREAD;
            }
        }

        // Thread has been terminated either from queue or waiter wakeup
        tid_t finished_tid = finished->tid;
        if(status != NULL) {
            *status = (int)finished->status;
        }

        // Remove terminated context from global
        remove_from_all_threads(finished);

        // Free stack if allocated
        if (finished->stack != NULL) {
            free_stack(finished->stack, finished->stacksize);
        }
        
        free(finished);
        return finished_tid;
    }

    scheduler lwp_get_scheduler(void) {
        ensure_scheduler();
        return current_scheduler;
    }

    void lwp_set_scheduler(scheduler sched) {
        scheduler old_sched;
        scheduler new_sched;
        thread t;

        // Make sure default exists
        ensure_scheduler();
        old_sched = current_scheduler;

        // NULL means switch back to default RR
        if (sched == NULL) {
            new_sched = RoundRobin;
        } else {
            new_sched = sched;
        }

        // return if requested sched is active
        if (new_sched == old_sched) {
            return;
        }

        // initialize new scheduler
        if (new_sched->init != NULL) {
            new_sched->init();
        }

        // transfer threads
        while (old_sched->qlen() > 0) {
            t = old_sched->next();
            if (t == NULL ){
                abort();
            }
            old_sched->remove(t);
            new_sched->admit(t);
        }

        // shut down old
        if (old_sched->shutdown != NULL) {
            old_sched->shutdown();
        }

        // publish new
        current_scheduler = new_sched;
    }