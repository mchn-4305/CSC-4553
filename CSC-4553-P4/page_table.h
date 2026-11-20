#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "config.h"

typedef struct {
    int page_to_frame[NUM_PAGES];
} PageTable;

void pt_init(PageTable *pt);
int pt_lookup(PageTable *pt, int page);
void pt_set(PageTable *pt, int page, int frame);
void pt_invalidate(PageTable *pt, int page);

#endif
