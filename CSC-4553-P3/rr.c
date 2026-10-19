#include <stddef.h>
#include "lwp.h"

static thread head = NULL;
static int thread_count = 0;

static void rr_admit(thread t) {
  if (head == NULL) {
    head = t;
    t->sched_one = t->sched_two = t;
    } else {
        thread tail = head->sched_two;
        tail->sched_one = t;
        t->sched_two = tail;
        t->sched_one = head;
        head->sched_two = t;
    }
    thread_count++;
}

static void rr_remove(thread t) {
    if (thread_count == 0) {
        return;
    }
    if (thread_count == 1) {
        head = NULL;
    } else {
        t->sched_two->sched_one = t->sched_one;
        t->sched_one->sched_two = t->sched_two;
        if (head == t) {
            head = t->sched_one;
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
