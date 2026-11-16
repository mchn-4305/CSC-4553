#include <stdio.h>
#include <stdlib.h>
#include "config.h"


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
            if (cap != 0) {
                cap *= 2;
            } else {
                cap = 1024;
            }
            addrs = realloc(addrs, cap * sizeof(int));
            if (addrs == NULL) {
                fclose(fp);
                return NULL;
            }
        }
        addrs[n++] = (int)value;
    }
    fclose(fp);

    *count = n;
    return addrs;
}

/*Usage: ./memsimulator <reference-sequence-file.txt> <FRAMES> <PRA>*/
int main(int argc, char *argv[]) {
    int *addrs;
    int n = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./memsimulator <reference-sequence-file.txt> <FRAMES> <PRA>\n");
        return 1;
    }

    addrs = read_refs(argv[1], &n);
    if (addrs == NULL) {
        perror(argv[1]);
        return 1;
    }

    int i;
    for (i = 0; i < n; i++) {
        int logical = addrs[i] & ADDRESS_MASK;
        int page = logical >> PAGE_SHIFT;
        int offset = logical & OFFSET_MASK;

        printf("addr %d -> page %d, offset %d\n", addrs[i], page, offset);
    }
    free(addrs);
    return 0;
}