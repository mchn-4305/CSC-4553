
#ifndef TLB_H
#define TLB_H

#include "config.h"

typedef struct {
    int pages[TLB_SIZE];
    int frames[TLB_SIZE];
    int next_slot;
    int hits;
    int misses;
} TLB;

void tlb_init(TLB *tlb);
int tlb_lookup(TLB *tlb, int page);
void tlb_insert(TLB *tlb, int page, int frame);

#endif