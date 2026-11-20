#include "phys_mem.h"

void pm_init(PhysMem *pm, int num_frames) {
    int i;

    pm->num_frames = num_frames;
    pm->next_available = 0;
    pm->fifo_next = 0;
    pm->access_clock = 0;

    for (i = 0; i < MAX_FRAME_SIZE; i++) {
        pm->frame_to_page[i] = -1;
        pm->last_used[i] = 0;
    }
}

int pm_get_frame(PhysMem *pm) {
    if (pm->next_available < pm->num_frames) {
        return pm->next_available++;
    }

    return -1;
}

int pm_fifo_victim(PhysMem *pm, int *victim_page) {
    int frame;

    if (pm->next_available < pm->num_frames) {
        return -1;
    }

    frame = pm->fifo_next;
    *victim_page = pm->frame_to_page[frame];

    pm->fifo_next =
        (pm->fifo_next + 1) % pm->num_frames;

    return frame;
}

int pm_lru_victim(PhysMem *pm, int *victim_page) {
    int i;
    int victim = -1;

    if (pm->next_available < pm->num_frames) {
        return -1;
    }

    for (i = 0; i < pm->num_frames; i++) {
        if (pm->frame_to_page[i] >= 0 && (victim < 0 || pm->last_used[i] < pm->last_used[victim])) {
            victim = i;
        }
    }

    if (victim >= 0) {
        *victim_page = pm->frame_to_page[victim];
    }

    return victim;
}

void pm_touch(PhysMem *pm, int frame) {
    pm->access_clock++;
    pm->last_used[frame] = pm->access_clock;
}

void pm_set_page(PhysMem *pm, int frame, int page) {
    pm->frame_to_page[frame] = page;
}

signed char *pm_frame(PhysMem *pm, int frame) {
    return &pm->memory[frame * PAGE_SIZE];
}

signed char pm_read(PhysMem *pm, int frame, int offset) {
    return pm->memory[frame * PAGE_SIZE + offset];
}