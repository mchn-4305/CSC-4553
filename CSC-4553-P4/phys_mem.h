#ifndef PHYS_MEM_H
#define PHYS_MEM_H
#include "config.h"

typedef struct {
    signed char memory[MAX_FRAME_SIZE * PAGE_SIZE];
    int num_frames;
    int next_available;
} PhysMem;


void pm_init(PhysMem *pm, int num_frames);
int pm_get_frame(PhysMem *pm);
signed char *pm_frame(PhysMem *pm, int frame);
signed char pm_read(PhysMem *pm, int frame, int offset);

#endif