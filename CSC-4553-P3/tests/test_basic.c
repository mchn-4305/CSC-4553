#include <stdio.h>
#include "../lwp.h"

static int num(void * arg) {
    char *name = arg;

    for (int i = 1; i <= 5; i++) {
        printf("Thread: %s: %d\n", name, i);
        lwp_yield();
    }
    return 0;
}

int main(void) {
    lwp_create(num, "A");
    lwp_create(num, "B");
    lwp_create(num, "C");
    printf("Expecting interleaved output from threads A, B, and C\n");
    printf("Starting\n");
    lwp_start();
    lwp_exit(0);
}