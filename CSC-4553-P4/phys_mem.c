#include "phys_mem.h"

void pm_init(PhysMem *pm, int num_frames) {
    pm->num_frames = num_frames;
    pm->next_available = 0;
}

int pm_get_frame(PhysMem *pm) {
    if (pm->next_available < pm->num_frames) {
        return pm->next_available++;
    }
    return -1;
}

signed char *pm_frame(PhysMem *pm, int frame) {
    return &pm->memory[frame * PAGE_SIZE];
}

signed char pm_read(PhysMem *pm, int frame, int offset) {
    return pm->memory[frame * PAGE_SIZE + offset];
}