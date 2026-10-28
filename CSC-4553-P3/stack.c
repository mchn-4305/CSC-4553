
#define _GNU_SOURCE

#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

#include "stack.h"

#define DEFAULT_STACK_SIZE (8UL * 1024 * 1024)

void *allocate_stack(size_t *stack_size)
{
    if (stack_size == NULL) {
        return NULL;
    }

    *stack_size = 0;

    // Determine the system page size.
    long page = sysconf(_SC_PAGE_SIZE);

    if (page <= 0) {
        return NULL;
    }

    size_t page_size = (size_t) page;

    // Determine the desired stack size.
    size_t size = DEFAULT_STACK_SIZE;

    struct rlimit limit;

    if (getrlimit(RLIMIT_STACK, &limit) == 0 &&
        limit.rlim_cur != RLIM_INFINITY) {

        // Ensure the limit fits into size_t.
        if (limit.rlim_cur > 0 &&
            (uintmax_t) limit.rlim_cur <= SIZE_MAX) {
            size = (size_t) limit.rlim_cur;
        }
    }

    // Round upward to a page boundary.
    size_t remainder = size % page_size;

    if (remainder != 0) {
        size_t extra = page_size - remainder;

        // Prevent arithmetic overflow.
        if (size > SIZE_MAX - extra) {
            return NULL;
        }

        size += extra;
    }

    // Allocate the stack mapping.
    void *stack = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
        -1,
        0
    );

    // Check for allocation failure.
    if (stack == MAP_FAILED) {
        return NULL;
    }

    *stack_size = size;

    return stack;
}

void free_stack(void *stack, size_t stack_size)
{
    if (stack != NULL && stack_size > 0) {
        munmap(stack, stack_size);
    }
}