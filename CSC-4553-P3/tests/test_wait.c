
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../lwp.h"

static int immediate_worker(void *arg)
{
    int value = *(int *)arg;
    return value;
}

static int delayed_worker(void *arg)
{
    int value = *(int *)arg;

    printf("Worker: yielding before termination\n");
    lwp_yield();

    printf("Worker: now terminating\n");
    return value;
}

static void test_fifo(void)
{
    int a = 10;
    int b = 300;
    int c = 30;

    tid_t ta = lwp_create(immediate_worker, &a);
    tid_t tb = lwp_create(immediate_worker, &b);
    tid_t tc = lwp_create(immediate_worker, &c);

    assert(ta != NO_THREAD);
    assert(tb != NO_THREAD);
    assert(tc != NO_THREAD);

    /*
     * Workers A, B, and C are ahead of main
     * in the Round Robin queue, and they
     * terminate without yielding.
     */
    lwp_start();

    int status;

    assert(lwp_wait(&status) == ta);
    assert(LWPTERMINATED(status));
    assert(LWPTERMSTAT(status) == 10);
    assert(tid2thread(ta) == NULL);

    printf("PASS: First terminated thread collected\n");

    assert(lwp_wait(&status) == tb);
    assert(LWPTERMINATED(status));
    assert(LWPTERMSTAT(status) == 44);
    assert(tid2thread(tb) == NULL);

    printf("PASS: Exit status truncated to 8 bits\n");

    assert(lwp_wait(NULL) == tc);
    assert(tid2thread(tc) == NULL);

    printf("PASS: NULL status argument\n");

    assert(lwp_wait(&status) == NO_THREAD);

    printf("PASS: No remaining thread to collect\n");
    printf("ALL FIFO TESTS PASSED!\n");
}

static void test_block(void)
{
    int value = 77;

    tid_t tid = lwp_create(delayed_worker, &value);

    assert(tid != NO_THREAD);

    /*
     * The worker yields before returning.
     * Main then reaches lwp_wait() while
     * the worker is still alive.
     */
    lwp_start();

    printf("Main: calling lwp_wait()\n");

    int status = -1;
    tid_t finished = lwp_wait(&status);

    assert(finished == tid);
    assert(LWPTERMINATED(status));
    assert(LWPTERMSTAT(status) == 77);
    assert(tid2thread(tid) == NULL);

    printf("PASS: Main resumed after blocking\n");

    assert(lwp_wait(NULL) == NO_THREAD);

    printf("ALL BLOCKING TESTS PASSED!\n");
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s fifo|block\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "fifo") == 0) {
        test_fifo();
    } else if (strcmp(argv[1], "block") == 0) {
        test_block();
    } else {
        fprintf(stderr, "Unknown test mode\n");
        return 1;
    }

    return 0;
}
