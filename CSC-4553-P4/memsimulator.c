#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "backing_store.h"
#include "page_table.h"

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
    PageTable pt;

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

    for (i = 0; i < n; i++) {
        int logical = addrs[i] & ADDRESS_MASK;
        int page = logical >> PAGE_SHIFT;
        int offset = logical & OFFSET_MASK;
        int frame = pt_resolve(&pt, page);
        int physical;

        if (frame < 0) {
            fprintf(stderr, "Error loading page %d\n", page);
            result = 1;
            break;
        }
        physical = frame * PAGE_SIZE + offset;
        printf("Virtual address: %d Physical address: %d Value: %d\n",
               addrs[i], physical, (int)pt.memory[physical]);
    }

    if (result == 0) {
        printf("Number of Translated Addresses = %d\n", n);
        printf("Page Faults = %d\n", pt.page_faults);
        printf("Page Fault Rate = %.3f\n", n ? (double)pt.page_faults / n : 0.0);
    }

    bs_close();
    free(addrs);
    return result;
}