
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../rr.h"

int main(void) {
    // Create four dummy threads.
    context a, b, c, d;

    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    memset(&c, 0, sizeof(c));
    memset(&d, 0, sizeof(d));

    a.tid = 1;
    b.tid = 2;
    c.tid = 3;
    d.tid = 4;

    // TEST 1: Empty scheduler
    printf("Test 1: Empty scheduler\n");

    assert(RoundRobin->qlen() == 0);
    assert(RoundRobin->next() == NULL);

    printf("PASS\n\n");

    // TEST 2: One thread
    printf("Test 2: One thread\n");

    RoundRobin->admit(&a);

    assert(RoundRobin->qlen() == 1);
    assert(RoundRobin->next() == &a);
    assert(RoundRobin->next() == &a);

    printf("PASS\n\n");

    // TEST 3: Round Robin ordering
    printf("Test 3: Round Robin ordering\n");

    RoundRobin->admit(&b);
    RoundRobin->admit(&c);

    assert(RoundRobin->qlen() == 3);

    assert(RoundRobin->next() == &a);
    assert(RoundRobin->next() == &b);
    assert(RoundRobin->next() == &c);
    assert(RoundRobin->next() == &a);

    printf("PASS\n\n");

    // TEST 4: Remove middle thread
    printf("Test 4: Remove middle thread\n");

    RoundRobin->remove(&b);

    assert(RoundRobin->qlen() == 2);
    assert(RoundRobin->next() == &c);
    assert(RoundRobin->next() == &a);
    assert(RoundRobin->next() == &c);

    printf("PASS\n\n");

    // TEST 5: Add another thread
    printf("Test 5: Add another thread\n");

    RoundRobin->admit(&d);

    assert(RoundRobin->qlen() == 3);
    assert(RoundRobin->next() == &a);
    assert(RoundRobin->next() == &d);
    assert(RoundRobin->next() == &c);

    printf("PASS\n\n");

    // TEST 6: Remove head
    printf("Test 6: Remove head\n");

    RoundRobin->remove(&a);

    assert(RoundRobin->qlen() == 2);
    assert(RoundRobin->next() == &d);
    assert(RoundRobin->next() == &c);

    printf("PASS\n\n");

    // TEST 7: Remove tail
    printf("Test 7: Remove tail\n");

    // Current head is d, so c is the tail.
    RoundRobin->remove(&c);

    assert(RoundRobin->qlen() == 1);
    assert(RoundRobin->next() == &d);

    printf("PASS\n\n");

    // TEST 8: Remove last thread
    printf("Test 8: Remove last thread\n");

    RoundRobin->remove(&d);

    assert(RoundRobin->qlen() == 0);
    assert(RoundRobin->next() == NULL);

    printf("PASS\n\n");

    // TEST 9: Re-admit after emptying
    printf("Test 9: Re-admit after emptying\n");

    RoundRobin->admit(&b);

    assert(RoundRobin->qlen() == 1);
    assert(RoundRobin->next() == &b);

    RoundRobin->remove(&b);

    assert(RoundRobin->qlen() == 0);

    printf("PASS\n\n");

    printf("ALL ROUND ROBIN TESTS PASSED!\n");

    return 0;
}