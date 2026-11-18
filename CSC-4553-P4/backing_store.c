#include <stdio.h>
#include "backing_store.h"
#include "config.h"

static FILE *store;

int bs_open(const char *path) {
    store = fopen(path, "rb");
    if (store == NULL) {
        return -1;
    }
    return 0;
}

int bs_read_page(int page, signed char *destination) {
    if (fseek(store, (long)page * PAGE_SIZE, SEEK_SET) != 0) {
        return -1;
    }
    if (fread(destination, 1, PAGE_SIZE, store) != PAGE_SIZE) {
        return -1;
    }
    return 0;
}

void bs_close(void) {
    if (store != NULL) {
        fclose(store);
        store = NULL;
    }
}