
#include <assert.h>
#include <stdio.h>

#include "../lwp.h"

static int worker(void *arg)
{
    (void)arg;

    tid_t my_tid = lwp_gettid();

    assert(my_tid != NO_THREAD);

    thread me = tid2thread(my_tid);

    assert(me != NULL);
    assert(me->tid == my_tid);

    printf("Worker TID %lu verified\n", my_tid);

    return 0;
}

int main(void)
{
    /* Before LWP system starts */
    assert(lwp_gettid() == NO_THREAD);
    assert(tid2thread(NO_THREAD) == NULL);

    printf("PASS: No active LWP before start\n");

    tid_t a = lwp_create(worker, NULL);
    tid_t b = lwp_create(worker, NULL);
    tid_t c = lwp_create(worker, NULL);

    assert(a != NO_THREAD);
    assert(b != NO_THREAD);
    assert(c != NO_THREAD);

    assert(a != b && b != c && a != c);

    printf("PASS: Unique thread IDs\n");

    /* Lookup works before threads execute */
    assert(tid2thread(a) != NULL);
    assert(tid2thread(b) != NULL);
    assert(tid2thread(c) != NULL);

    assert(tid2thread(a)->tid == a);
    assert(tid2thread(b)->tid == b);
    assert(tid2thread(c)->tid == c);

    assert(tid2thread(999999) == NULL);

    printf("PASS: Thread lookup before execution\n");

    lwp_start();

    printf("Main resumed with TID %lu\n", lwp_gettid());

    /*
     * End main's LWP so the workers can finish.
     * Once lwp_wait() exists, we'll collect them
     * properly rather than exiting main here.
     */
    lwp_exit(0);

    return 0;
}