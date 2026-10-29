
#include <stdio.h>
#include "../lwp.h"

static int num(void *arg)
{
    char *name = arg;

    for (int i = 1; i <= 5; i++) {
        printf("Thread %s: %d\n", name, i);
        lwp_yield();
    }

    return 0;
}

int main(void)
{
    lwp_create(num, "A");
    lwp_create(num, "B");
    lwp_create(num, "C");

    printf("Starting LWP system\n");

    lwp_start();

    printf("Main resumed successfully!\n");

    // Main exits, allowing workers to finish.
    lwp_exit(0);
}
