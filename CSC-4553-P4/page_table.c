#include "page_table.h"

void pt_init(PageTable *pt) {
    int page;
    for (page = 0; page < NUM_PAGES; page++) {
        pt->page_to_frame[page] = -1;
    }
}

int pt_lookup(PageTable *pt, int page) {
    return pt->page_to_frame[page];
}

void pt_set(PageTable *pt, int page, int frame) {
    pt->page_to_frame[page] = frame;
}

void pt_invalidate(PageTable *pt, int page) {
    pt->page_to_frame[page] = -1;
}
