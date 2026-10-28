
#ifndef LWP_STACK_H
#define LWP_STACK_H

#include <stddef.h>

/*
 * Allocate a stack using mmap().
 *
 * On success:
 *   Returns the base of the mapping.
 *   Stores the allocation size in *stack_size.
 *
 * On failure:
 *   Returns NULL.
 *   Sets *stack_size to 0.
 */
void *allocate_stack(size_t *stack_size);

/*
 * Release a previously allocated stack.
 */
void free_stack(void *stack, size_t stack_size);

#endif