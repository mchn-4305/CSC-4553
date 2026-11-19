#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "config.h"

typedef struct {
    int page_to_frame[NUM_PAGES];
    signed char memory[NUM_PAGES * PAGE_SIZE];
    int next_free_frame;
    int page_faults;
} PageTable;

void pt_init(PageTable *pt);
// Return the physical frame for page, or -1 on backing-store error.
int pt_resolve(PageTable *pt, int page);

#endif
