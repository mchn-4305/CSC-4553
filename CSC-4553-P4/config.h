#ifndef CONFIG_H
#define CONFIG_H

#define PAGE_SIZE 256
#define NUM_PAGES 256
#define TLB_SIZE 16
#define MAX_FRAME_SIZE 256

#define OFFSET_MASK 0xFF
#define PAGE_SHIFT 8
#define ADDRESS_MASK 0xFFFF

#define BACKING_STORE_FILE "./files/BACKING_STORE.bin"

#endif