
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <stdint.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define BUFF_SIZE (1<<21)

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

int main(int argc, char **argv)
{
	// Open the covert channel
	int fd = shm_open("/covert_channel", O_RDWR, 0666);
	if (fd == -1) {
		perror("shm_open() error - make sure sender is running first\n");
		exit(EXIT_FAILURE);
	}

	char *buf = mmap(NULL, BUFF_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED, fd, 0);
  
    if (buf == (void*) - 1) {
     perror("mmap() error\n");
     exit(EXIT_FAILURE);
    }

	// Setup synchronization
	sync_t *sync = (sync_t *)buf;
	
	// Target address for covert channel (same offset as sender)
	char *target = buf + sizeof(sync_t);

	printf("Please press enter.\n");

	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);

	printf("Receiver now listening.\n");

	bool listening = true;
	int bit = -1;
	int latency;
	int total_latency = 0;
	int measurement_count = 0;

	while (listening) {
		// Wait for sender to prepare the cache state
		while (!sync->sender_ready) {
			usleep(10000); // Sleep 10ms between checks
		}

		printf("Sender ready, measuring cache state...\n");

		// Give a brief moment for cache state to stabilize
		delay(1);

		// Take multiple measurements during the measurement window
		total_latency = 0;
		measurement_count = 0;
		
		for (int measurement = 0; measurement < 20; measurement++) {
			// Flush L1 to ensure we measure from L2/L3
			flush_cache_line(target);
			
			// Small delay for cache coherency
			delay(0);
			
			// Take multiple access time samples
			for (int i = 0; i < 100; i++) {
				total_latency += measure_one_block_access_time((uint64_t)target);
				measurement_count++;
			}
			
			// Wait a bit before next measurement batch
			delay(0);
		}

		latency = total_latency / measurement_count;

		// Threshold calibration:
		// L2 hit: ~10-15 cycles (~40-60 ns)
		// L3 hit: ~40 cycles (~160 ns)  
		// DRAM: ~100+ cycles (~400+ ns)
		// Conservative threshold: 200 cycles
		if (latency > 110) { 
			// Miss to L3/DRAM --> bit is 0
			bit = 0;
		} else { 
			// Hit in L2 --> bit is 1
			bit = 1;
		}
		
		printf("\nBit: %d\n", bit);
		printf("Average Latency: %d (from %d measurements)\n", latency, measurement_count);
		printf("Receiver finished.\n");

		listening = false;
	}

	munmap(buf, BUFF_SIZE);
	close(fd);
	return 0;
}



