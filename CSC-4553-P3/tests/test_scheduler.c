
#include <assert.h>
#include <stdio.h>

#include "../lwp.h"
#include "../rr.h"

/*
 * Simple alternative scheduler:
 * circular singly linked list.
 */
static thread alt_head = NULL;
static thread alt_tail = NULL;
static int alt_count = 0;

static void alt_init(void)
{
    alt_head = NULL;
    alt_tail = NULL;
    alt_count = 0;
}

static void alt_admit(thread t)
{
    if (alt_head == NULL) {
        alt_head = t;
        alt_tail = t;
        t->sched_one = t;
    } else {
        t->sched_one = alt_head;
        alt_tail->sched_one = t;
        alt_tail = t;
    }

    alt_count++;
}

static void alt_remove(thread victim)
{
    if (alt_count == 0) {
        return;
    }

    thread previous = alt_tail;
    thread cursor = alt_head;

    for (int i = 0; i < alt_count; i++) {
        if (cursor == victim) {
            if (alt_count == 1) {
                alt_head = NULL;
                alt_tail = NULL;
            } else {
                previous->sched_one = cursor->sched_one;

                if (cursor == alt_head) {
                    alt_head = cursor->sched_one;
                }

                if (cursor == alt_tail) {
                    alt_tail = previous;
                }
            }

            victim->sched_one = NULL;
            alt_count--;
            return;
        }

        previous = cursor;
        cursor = cursor->sched_one;
    }
}

static thread alt_next(void)
{
    if (alt_head == NULL) {
        return NULL;
    }

    thread result = alt_head;
    alt_head = alt_head->sched_one;
    alt_tail = result;

    return result;
}

static int alt_qlen(void)
{
    return alt_count;
}

static void alt_shutdown(void)
{
    assert(alt_count == 0);
    alt_head = NULL;
    alt_tail = NULL;
}

static struct scheduler alt_struct = {
    alt_init,
    alt_shutdown,
    alt_admit,
    alt_remove,
    alt_next,
    alt_qlen
};

static scheduler Alternative = &alt_struct;

static int worker(void *arg)
{
    int value = *(int *)arg;

    printf("Worker %d started\n", value);
    lwp_yield();

    printf("Worker %d resumed\n", value);
    return value;
}

int main(void)
{
    int a = 10;
    int b = 20;
    int status;

    assert(lwp_get_scheduler() == RoundRobin);

    tid_t ta = lwp_create(worker, &a);
    tid_t tb = lwp_create(worker, &b);

    assert(ta != NO_THREAD);
    assert(tb != NO_THREAD);

    lwp_start();

    /* Workers have each yielded once. */
    printf("Main: switching scheduler\n");

    assert(RoundRobin->qlen() == 3);

    lwp_set_scheduler(Alternative);

    assert(lwp_get_scheduler() == Alternative);
    assert(Alternative->qlen() == 3);
    assert(RoundRobin->qlen() == 0);

    printf("PASS: Changed to alternative scheduler\n");

    /* Reinstalling the same scheduler must be safe. */
    lwp_set_scheduler(Alternative);
    assert(Alternative->qlen() == 3);

    printf("PASS: Same-scheduler request is safe\n");

    /* Return to the default scheduler. */
    lwp_set_scheduler(NULL);

    assert(lwp_get_scheduler() == RoundRobin);
    assert(RoundRobin->qlen() == 3);
    assert(Alternative->qlen() == 0);

    printf("PASS: Restored Round Robin scheduler\n");

    /* Collect both workers. */
    assert(lwp_wait(&status) == ta);
    assert(LWPTERMSTAT(status) == 10);

    assert(lwp_wait(&status) == tb);
    assert(LWPTERMSTAT(status) == 20);

    assert(lwp_wait(NULL) == NO_THREAD);

    printf("ALL SCHEDULER TESTS PASSED!\n");

    return 0;
}
