
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

// TODO: define your own buffer size
#define BUFF_SIZE (1<<21)

#define L1_SIZE 32768
#define L2_SIZE 1048576

#define LINE_SIZE 64

// Synchronization structure in shared memory
typedef struct {
    volatile int sender_ready;
    volatile int bit_value;
} sync_t;

void delay (int seconds) {
	long pause = seconds * CLOCKS_PER_SEC;
	clock_t start = clock();
	while (clock() - start < pause);
}

// Explicitly flush a cache line
static inline void flush_cache_line(void *addr) {
    asm volatile("clflush (%0)" : : "r" (addr));
}

// Prefetch into cache
static inline void prefetch_cache_line(void *addr) {
    asm volatile("prefetcht0 (%0)" : : "r" (addr));
}

int main(int argc, char **argv)
{

    int fd = shm_open("/covert_channel", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, BUFF_SIZE);

    // Allocate a buffer using huge page
    // See the handout for details about hugepage management
    char *buf = mmap(NULL, BUFF_SIZE,
        PROT_READ|PROT_WRITE,
        MAP_SHARED, fd, 0);
  
    if (buf == (void*) - 1) {
     perror("mmap() error\n");
     exit(EXIT_FAILURE);
    }

    
    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)buf) = 1; // dummy write to trigger page allocation

    volatile char tmp;
    volatile int dummy;

    // Setup synchronization in first part of shared buffer
    sync_t *sync = (sync_t *)buf;
    memset(buf, 0, BUFF_SIZE);
    
    // Target address for covert channel (offset from sync structure)
    char *target = buf + sizeof(sync_t);

    printf("Please type a message.\n");

    bool sending = true;
    while (sending) {
        char text_buf[128];
        fgets(text_buf, sizeof(text_buf), stdin);

        char c = text_buf[0];
        int transmit_bit = c & 0b1;

        printf("Transmitting bit: %d\n", transmit_bit);

        if (transmit_bit == 1) { 
            // Bit = 1: Keep target in L2 cache
            // Repeatedly load to ensure L2 residency
            for (int i = 0; i < 5000; i++) {
                tmp = *target;
            }
        } else { 
            // Bit = 0: Flush target to L3/DRAM
            // Use clflush to explicitly remove from L1/L2
            flush_cache_line(target);
            flush_cache_line(target + 64);
        }

        // Signal receiver that bit has been set
        sync->bit_value = transmit_bit;
        sync->sender_ready = 1;

        // Maintain cache state for measurement window (2 seconds)
        clock_t start = clock();
        while (clock() - start < 2 * CLOCKS_PER_SEC) {
            if (transmit_bit == 1) {
                // Keep in L2: repeated access
                for (int i = 0; i < 100; i++) {
                    tmp = *target;
                }
            } else {
                // Keep evicted: flush periodically to prevent natural reload
                if ((clock() - start) % (CLOCKS_PER_SEC / 10) == 0) {
                    flush_cache_line(target);
                }
            }
        }

        sync->sender_ready = 0;
        sending = false;
    }

  printf("Sender finished.\n");
  munmap(buf, BUFF_SIZE);
  shm_unlink("/covert_channel");
  return 0;
}


