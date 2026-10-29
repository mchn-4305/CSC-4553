
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lwp.h"
#include "../rr.h"
#include "../stack.h"

/*
 * Temporary stub for testing.
 * Remove this when testing the complete library.
 */
void lwp_exit(int status)
{
    (void)status;
    abort();
}

static int dummy_function(void *arg)
{
    return *(int *)arg;
}

int main(void)
{
    int a = 10;
    int b = 20;

    /* Invalid function pointer */
    assert(lwp_create(NULL, NULL) == NO_THREAD);
    printf("PASS: NULL function rejected\n");

    /* Create two threads */
    tid_t tid1 = lwp_create(dummy_function, &a);
    tid_t tid2 = lwp_create(dummy_function, &b);

    assert(tid1 != NO_THREAD);
    assert(tid2 != NO_THREAD);
    assert(tid1 != tid2);

    printf("PASS: Unique nonzero thread IDs\n");

    /* Both threads should be runnable. */
    assert(RoundRobin->qlen() == 2);

    thread t1 = RoundRobin->next();
    thread t2 = RoundRobin->next();

    assert(t1 != NULL);
    assert(t2 != NULL);

    assert(t1->tid == tid1);
    assert(t2->tid == tid2);

    printf("PASS: Threads admitted in creation order\n");

    /* Verify stack allocations. */
    assert(t1->stack != NULL);
    assert(t2->stack != NULL);
    assert(t1->stacksize > 0);
    assert(t2->stacksize > 0);

    printf("PASS: Private stacks allocated\n");

    /* Verify function arguments. */
    assert(t1->state.rdi ==
           (unsigned long)(uintptr_t)dummy_function);
    assert(t1->state.rsi ==
           (unsigned long)(uintptr_t)&a);

    assert(t2->state.rdi ==
           (unsigned long)(uintptr_t)dummy_function);
    assert(t2->state.rsi ==
           (unsigned long)(uintptr_t)&b);

    printf("PASS: Function arguments initialized\n");

    /* Verify initial stack alignment. */
    assert(t1->state.rsp % 16 == 8);
    assert(t2->state.rsp % 16 == 8);

    /*
     * After leave; ret, the wrapper's entry
     * stack pointer must also be 8 mod 16.
     */
    assert((t1->state.rbp + 16) % 16 == 8);
    assert((t2->state.rbp + 16) % 16 == 8);

    printf("PASS: Initial stack alignment\n");

    /* Clean up test allocations. */
    RoundRobin->remove(t1);
    RoundRobin->remove(t2);

    free_stack(t1->stack, t1->stacksize);
    free_stack(t2->stack, t2->stacksize);

    free(t1);
    free(t2);

    printf("\nALL LWP CREATE TESTS PASSED!\n");

    return 0;
}
