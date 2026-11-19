#include "page_table.h"
#include "backing_store.h"

void pt_init(PageTable *pt) {
    int page;
    for (page = 0; page < NUM_PAGES; page++) {
        pt->page_to_frame[page] = -1;
    }
    pt->next_free_frame = 0;
    pt->page_faults = 0;
}

int pt_resolve(PageTable *pt, int page) {
    int frame;

    if (page < 0 || page >= NUM_PAGES) {
        return -1;
    }
    frame = pt->page_to_frame[page];
    if (frame != -1) {
        return frame;
    }
    if (pt->next_free_frame >= NUM_PAGES) {
        return -1;
    }

    frame = pt->next_free_frame;
    if (bs_read_page(page, &pt->memory[frame * PAGE_SIZE]) != 0) {
        return -1;
    }

    pt->page_to_frame[page] = frame;
    pt->next_free_frame++;
    pt->page_faults++;
    return frame;
}
