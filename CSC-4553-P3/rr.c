#include <stddef.h>
#include "lwp.h"

static thread head = NULL;
static int thread_count = 0;

static void rr_admit(thread new) {
  if (head == NULL) {
    head = new;
    new->sched_one = new->sched_two = new;
    } else {
        thread tail = head->sched_two;
        tail->sched_one = new;
        new->sched_two = tail;
        new->sched_one = head;
        head->sched_two = new;
    }
    thread_count++;
}

static void rr_remove(thread victim) {
    if (thread_count == 0) {
        return;
    }
    if (thread_count == 1) {
        head = NULL;
    } else {
        victim->sched_two->sched_one = victim->sched_one;
        victim->sched_one->sched_two = victim->sched_two;
        if (head == victim) {
            head = victim->sched_one;
        }
    }
    thread_count--;
}

static thread rr_next() {
    if (head == NULL) {
        return NULL;
    }
    thread t = head;
    head = head->sched_one;
    return t;
}

static int rr_count() {
    return thread_count;
}

struct scheduler rr_scheduler = {
    NULL, NULL, rr_admit, rr_remove, rr_next, rr_count
};

scheduler RoundRobin = &rr_scheduler;
