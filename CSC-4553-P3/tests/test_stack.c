
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

#include "../stack.h"

int main(void)
{
    size_t size1 = 0;
    size_t size2 = 0;

    long page = sysconf(_SC_PAGE_SIZE);
    assert(page > 0);

    printf("System page size: %ld bytes\n", page);

    // Test 1: Allocate a stack.
    void *stack1 = allocate_stack(&size1);

    assert(stack1 != NULL);
    assert(size1 > 0);

    printf("Stack 1 base: %p\n", stack1);
    printf("Stack 1 size: %zu bytes\n", size1);

    // Test 2: Verify page alignment.
    assert(size1 % (size_t)page == 0);
    printf("PASS: Page-aligned stack size\n");

    // Test 3: Verify requested stack size.
    struct rlimit limit;
    int result = getrlimit(RLIMIT_STACK, &limit);

    if (result == 0 &&
        limit.rlim_cur != RLIM_INFINITY &&
        limit.rlim_cur > 0 &&
        (uintmax_t)limit.rlim_cur <= SIZE_MAX) {

        assert(size1 >= (size_t)limit.rlim_cur);
    }

    printf("PASS: Stack size requirements\n");

    // Test 4: Write to the first and last bytes.
    unsigned char *memory = stack1;

    memory[0] = 42;
    memory[size1 - 1] = 99;

    assert(memory[0] == 42);
    assert(memory[size1 - 1] == 99);

    printf("PASS: Stack memory is readable/writable\n");

    // Test 5: Allocate another independent stack.
    void *stack2 = allocate_stack(&size2);

    assert(stack2 != NULL);
    assert(size2 > 0);
    assert(stack2 != stack1);

    unsigned char *memory2 = stack2;

    memory2[0] = 77;

    assert(memory[0] == 42);
    assert(memory2[0] == 77);

    printf("PASS: Separate stack allocations\n");

    // Test 6: Release both stacks.
    free_stack(stack1, size1);
    free_stack(stack2, size2);

    printf("PASS: Both stacks released\n");

    // Test 7: NULL output pointer.
    assert(allocate_stack(NULL) == NULL);

    printf("PASS: NULL argument handling\n");

    printf("\nALL STACK TESTS PASSED!\n");

    return 0;
}