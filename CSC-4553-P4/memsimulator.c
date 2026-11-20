#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "backing_store.h"
#include "page_table.h"
#include "phys_mem.h"

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

/* Milestone 2: only the 256-frame, no-replacement mode is implemented. */
int main(int argc, char *argv[]) {
    int *addrs;
    int n = 0;
    int i;
    int result = 0;
    int page_faults = 0;
    PageTable pt;
    static PhysMem pm;

    if (argc != 2) {
        fprintf(stderr, "Usage (milestone 2): %s <reference-sequence-file.txt>\n", argv[0]);
        return 1;
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

    // Change when we implement frames as an argument
    pm_init(&pm, MAX_FRAME_SIZE);

    for (i = 0; i < n; i++) {
        int logical = addrs[i] & ADDRESS_MASK;
        int page = logical >> PAGE_SHIFT;
        int offset = logical & OFFSET_MASK;
        int frame = pt_lookup(&pt, page);

        if (frame < 0) {
            page_faults++;
            frame = pm_get_frame(&pm);
            if (frame < 0) {
                fprintf(stderr, "Out of frames (replacement not implemented yet)\n");
                result = 1;
                break;
            }
            
            if (bs_read_page(page, pm_frame(&pm, frame)) != 0)  {
                fprintf(stderr, "Error loading page %d\n", page);
                result = 1;
                break;
            }
            pt_set(&pt, page, frame);
        }
        
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
        // TLB Info temporary (not implemented yet)
        printf("TLB Hits = %d\n", 0);
        printf("TLB Misses = %d\n", n);
        printf("TLB Hit Rate = %.3f\n", 0.0);

    }

    bs_close();
    free(addrs);
    return result;
}