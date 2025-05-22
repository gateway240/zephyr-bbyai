#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

// R5 Shared Memory:
// We have 32K (=0x8000) of BTCM ram. 
// Code running in Zephyr usess up to about address 0x069f8 (see below)
// To be safe, we use the last 4k of BTCM: 0x7000 - 0x8000.
/*
    $ readelf -l ./build/zephyr/zephyr.elf
    ...
    Program Headers:
    Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
    EXIDX          0x003a88 0x000039b0 0x000039b0 0x00008 0x00008 R   0x4
    LOAD           0x0000d8 0x00000000 0x00000000 0x0510c 0x069f8 RWE 0x8
    LOAD           0x0051e4 0x000069f8 0x000069f8 0x00070 0x00070 RW  0x4
    LOAD           0x005254 0xa1100000 0xa1100000 0x00010 0x00010 RW  0x4
    TLS            0x004b88 0x00004ab0 0x00004ab0 0x00000 0x00004 R   0x4

*/
#define MEM_START_OFFSET 0x7000

#define MSG_SIZE   32

#define MSG_OFFSET MEM_START_OFFSET
#define LED_DELAY_MS_OFFSET (MSG_OFFSET + MSG_SIZE)
#define IS_BUTTON_PRESSED_OFFSET (LED_DELAY_MS_OFFSET + sizeof(uint32_t))
#define BTN_COUNT_OFFSET (IS_BUTTON_PRESSED_OFFSET + sizeof(uint32_t))
#define LOOP_COUNT_OFFSET (BTN_COUNT_OFFSET + sizeof(uint32_t))
#define END_MEMORY_OFFSET (LOOP_COUNT_OFFSET + sizeof(uint32_t))

static inline uint8_t getSharedMem_uint8(volatile void *base, uint32_t byte_offset) {
    volatile uint8_t *addr_tmp = (uint8_t *)base + byte_offset ;
    volatile uint8_t val = *addr_tmp;
    return val;
}

static inline void setSharedMem_uint8(volatile void* base, uint32_t byte_offset, uint8_t val) {
    volatile uint8_t *addr_tmp = (uint8_t *)base + byte_offset;
    volatile uint8_t val_tmp = val;
    // printf("        --> Set8  addr 0x%08x to %x\n", (uint32_t *)addr_tmp, val_tmp);
    *addr_tmp = val_tmp;
}

static inline uint32_t getSharedMem_uint32(volatile void *base, uint32_t byte_offset) {
    volatile uint32_t *addr_tmp = (uint32_t *) ((uint8_t *)base + byte_offset);
    volatile uint32_t val = *addr_tmp;
    return val;
}

static inline void setSharedMem_uint32(volatile void* base, uint32_t byte_offset, uint32_t val) {
    volatile uint32_t *addr_tmp = (uint32_t *) ((uint8_t *)base + byte_offset);
    volatile uint32_t val_tmp = val;
    // printf("        --> Set32 addr 0x%08x to %x\n", (uint32_t *)addr_tmp, val_tmp);
    *addr_tmp = val_tmp;
}

// OLD: These are replaced by the above functions.
#define MEM_UINT8(addr) "ERROR DO NOT USE THIS"
#define MEM_UINT32(addr) "ERROR DO NOT USE THIS"


#endif
