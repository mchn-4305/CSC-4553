#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "backing_store.h"
#include "page_table.h"
#include "phys_mem.h"
#include "tlb.h"

static int *read_refs(const char *path, int *count) {
    FILE *fp;
    int *addrs = NULL;
    int cap = 0;
    int n = 0;
    long value;

    fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }

    while (fscanf(fp, "%ld", &value) == 1) {
        if (n == cap) {
            int new_cap = cap ? cap * 2 : 1024;
            int *temp = realloc(addrs, (size_t)new_cap * sizeof(*addrs));
            if (temp == NULL) {
                free(addrs);
                fclose(fp);
                return NULL;
            }
            addrs = temp;
            cap = new_cap;
        }
        addrs[n++] = (int)value;
    }

    fclose(fp);
    *count = n;
    return addrs;
}

int main(int argc, char *argv[]) {
    int *addrs;
    int n = 0;
    int i;
    int result = 0;
    int page_faults = 0;
    PageTable pt;
    TLB tlb;
    static PhysMem pm;
    int frames = MAX_FRAME_SIZE;
    // Temporary approach to policy choice
    // 0 = FIFO, 1 = LRU
    int use_lru = 0;

    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: %s <reference-file> [FRAMES] [fifo|lru]\n", argv[0]);
        return 1;
    }

    if (argc >= 3) {
        char *end;
        long parsed = strtol(argv[2], &end, 10);

        if (*argv[2] == '\0' ||
            *end != '\0' ||
            parsed < 1 ||
            parsed > MAX_FRAME_SIZE) {

            fprintf(stderr, "FRAMES must be an integer from 1 to %d\n", MAX_FRAME_SIZE);
            return 1;
        }

        frames = (int)parsed;
    }

    if (argc == 4) {
        if (strcmp(argv[3], "lru") == 0) {
            use_lru = 1;
        }
        else if (strcmp(argv[3], "fifo") != 0) {
            fprintf(stderr,
                "PRA must be fifo or lru\n");
            return 1;
        }
    }

    addrs = read_refs(argv[1], &n);
    if (addrs == NULL) {
        fprintf(stderr, "Could not read address file: %s\n", argv[1]);
        return 1;
    }

    if (bs_open(BACKING_STORE_FILE) != 0) {
        perror(BACKING_STORE_FILE);
        free(addrs);
        return 1;
    }

    pt_init(&pt);
    tlb_init(&tlb);

    // Change when we implement frames as an argument
    pm_init(&pm, frames);

    for (i = 0; i < n; i++) {
        int logical = addrs[i] & ADDRESS_MASK;
        int page = logical >> PAGE_SHIFT;
        int offset = logical & OFFSET_MASK;
        int frame = tlb_lookup(&tlb, page);

        if (frame < 0) {
            frame = pt_lookup(&pt, page);

            if (frame < 0) {
                int victim_page = -1;

                frame = pm_get_frame(&pm);

                if (frame < 0) {       
                    if (use_lru) {
                        frame = pm_lru_victim(&pm, &victim_page);
                    } else {
                        frame = pm_fifo_victim(&pm, &victim_page);
                    }
                }

                if (frame < 0) {
                    fprintf(stderr,
                        "Unable to allocate a frame\n");
                    result = 1;
                    break;
                }

                if (bs_read_page(page, pm_frame(&pm, frame)) != 0) {
                    fprintf(stderr,
                        "Error loading page %d\n", page);
                    result = 1;
                    break;
                }

                if (victim_page >= 0) {
                    pt_invalidate(&pt, victim_page);
                    tlb_invalidate(&tlb, victim_page);
                }

                pm_set_page(&pm, frame, page);
                pt_set(&pt, page, frame);
                page_faults++;
            }

            tlb_insert(&tlb, page, frame);
        }

        pm_touch(&pm, frame);
        signed char *bytes = pm_frame(&pm, frame);
        int b;
        printf("%d, %d, %d, ", addrs[i], (int)pm_read(&pm, frame, offset), frame);
        for (b = 0; b < PAGE_SIZE; b++) {
            printf("%02X", (unsigned char) bytes[b]);
        }
        printf("\n");
    }

    if (result == 0) {
        printf("Number of Translated Addresses = %d\n", n);
        printf("Page Faults = %d\n", page_faults);
        printf("Page Fault Rate = %.3f\n", n ? (double)page_faults / n : 0.0);
        printf("TLB Hits = %d\n", tlb.hits);
        printf("TLB Misses = %d\n", tlb.misses);
        printf("TLB Hit Rate = %.3f\n", n ? (double)tlb.hits / n : 0.0);

    }

    bs_close();
    free(addrs);
    return result;
}