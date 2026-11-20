
#include "tlb.h"

void tlb_init(TLB *tlb) {
    int i;

    for (i = 0; i < TLB_SIZE; i++) {
        tlb->pages[i] = -1;
        tlb->frames[i] = -1;
    }

    tlb->next_slot = 0;
    tlb->hits = 0;
    tlb->misses = 0;
}

int tlb_lookup(TLB *tlb, int page) {
    int i;

    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb->pages[i] == page) {
            tlb->hits++;
            return tlb->frames[i];
        }
    }

    tlb->misses++;
    return -1;
}

void tlb_insert(TLB *tlb, int page, int frame) {
    int i;

    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb->pages[i] == page) {
            tlb->frames[i] = frame;
            return;
        }
    }

    tlb->pages[tlb->next_slot] = page;
    tlb->frames[tlb->next_slot] = frame;

    tlb->next_slot = (tlb->next_slot + 1) % TLB_SIZE;
}