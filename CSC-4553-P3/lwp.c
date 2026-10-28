#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

#include "lwp.h"
#include "rr.h"

// The currently executing LWP
static thread current_thread = NULL;

// The original system thread (main program)
static thread main_thread = NULL;

// Next available thread ID
// Thread IDs must be unique and nonzero
static tid_t next_tid = 1;

// Active scheduling policy
// Initially, no scheduler has been explicitly selected
static scheduler current_scheduler = NULL;

// Head of the library's global thread list
static thread all_threads = NULL;

// FIFO queue of terminated threads
static thread terminated_head = NULL; // Oldest terminated thread
static thread terminated_tail = NULL; // Newest terminated thread