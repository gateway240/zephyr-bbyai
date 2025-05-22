#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <string.h>

#include "sharedDataLayout.h"

// General R5 Memomry Sharing Routine
// ----------------------------------------------------------------
#define ATCM_ADDR     0x79000000  // MCU ATCM (p59 TRM)
#define BTCM_ADDR     0x79020000  // MCU BTCM (p59 TRM)
#define MEM_LENGTH    0x8000

// Return the address of the base address of the ATCM memory region for the R5-MCU
volatile void* getR5MmapAddr(void)
{
    // Access /dev/mem to gain access to physical memory (for memory-mapped devices/specialmemory)
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem; Did you run with sudo?");
        exit(EXIT_FAILURE);
    }

    // Inside main memory (fd), access the part at offset BTCM_ADDR:
    // (Get points to start of R5 memory after it's memory mapped)
    volatile void* pR5Base = mmap(0, MEM_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BTCM_ADDR);
    if (pR5Base == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return pR5Base;
}

void freeR5MmapAddr(volatile void* pR5Base)
{
    if (munmap((void*) pR5Base, MEM_LENGTH)) {
        perror("R5 munmap failed");
        exit(EXIT_FAILURE);
    }
}

int main(void) 
{
    printf("Sharing memory with R5\n");
    printf("  LED should change speed every 5s.\n");
    printf("  Press the button to see its state here.\n");

    // Get access to shared memory for my uses
    volatile void *pR5Base = getR5MmapAddr();

	printf("Contents of memory :\n");
	for (int i = MSG_OFFSET; i < END_MEMORY_OFFSET; i++) {
        char val = getSharedMem_uint8(pR5Base, i);
        printf("Offset %d = %3d (char '%c')\n", i, val, val);
	}

    // Print out the mem contents:
    printf("From the R5, memory hold:\n");
    // NOTE: Cannot access it as a string, gives "Bus error"
    //printf("    %15s: \"%s\"\n", "msg", (char*)(pR5Base + MSG_OFFSET));
    printf("    %15s: 0x%04x\n", "ledDelay_ms", getSharedMem_uint32(pR5Base, LED_DELAY_MS_OFFSET));
    printf("    %15s: 0x%04x\n", "isButtonPressed", getSharedMem_uint32(pR5Base, IS_BUTTON_PRESSED_OFFSET));
    printf("    %15s: 0x%04x\n", "btnCount", getSharedMem_uint32(pR5Base, BTN_COUNT_OFFSET));
    printf("    %15s: 0x%04x\n", "loopCount", getSharedMem_uint32(pR5Base, LOOP_COUNT_OFFSET));

    // Drive it
    for (int i = 0; true; i++) {
        // Set LED timing
        setSharedMem_uint32(pR5Base, LED_DELAY_MS_OFFSET, (i % 10 < 5) ? 100 : 250);

        // Print button
        printf("Button: %15s   Btn Count: %7d    Loop Count: %7d\n", 
            getSharedMem_uint32(pR5Base, IS_BUTTON_PRESSED_OFFSET) ? "Pressed" : "Not pressed",
            getSharedMem_uint32(pR5Base, BTN_COUNT_OFFSET),
            getSharedMem_uint32(pR5Base, LOOP_COUNT_OFFSET)
        );

        // Timing
        sleep(1);
    }

    // Cleanup
    freeR5MmapAddr(pR5Base);
}