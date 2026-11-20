
#ifndef PHYS_MEM_H
#define PHYS_MEM_H

#include "config.h"

typedef struct {
    signed char memory[MAX_FRAME_SIZE * PAGE_SIZE];

    int num_frames;
    int next_available;

    int frame_to_page[MAX_FRAME_SIZE];
    int fifo_next;

    unsigned long long access_clock;
    unsigned long long last_used[MAX_FRAME_SIZE];
} PhysMem;

void pm_init(PhysMem *pm, int num_frames);
int pm_get_frame(PhysMem *pm);

int pm_fifo_victim(PhysMem *pm, int *victim_page);
int pm_lru_victim(PhysMem *pm, int *victim_page);
void pm_touch(PhysMem *pm, int frame);
void pm_set_page(PhysMem *pm, int frame, int page);

signed char *pm_frame(PhysMem *pm, int frame);
signed char pm_read(PhysMem *pm, int frame, int offset);

#endif