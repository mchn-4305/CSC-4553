#ifndef BACKING_STORE_H
#define BACKING_STORE_H

int bs_open(const char *path);
int bs_read_page(int page, signed char *destination);
void bs_close(void);

#endif