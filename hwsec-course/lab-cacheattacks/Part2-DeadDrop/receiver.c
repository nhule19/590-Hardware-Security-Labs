
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <stdint.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define BUFF_SIZE (1<<21)

int main(int argc, char **argv)
{
	// Put your covert channel setup code here
	int fd = shm_open("/covert_channel", O_CREAT | O_RDWR, 0666);
	char *buf = mmap(NULL, BUFF_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED, fd, 0);
  
    if (buf == (void*) - 1) {
     perror("mmap() error\n");
     exit(EXIT_FAILURE);
    }

	printf("Please press enter.\n");

	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);

	// printf("Receiver now listening.\n");

	bool listening = true;
	int bit;
	CYCLES latency;

	while (listening) {
		printf("Receiver now listening.\n");
		// Put your covert channel code here
		volatile char *flag = (volatile char *)&buf[4096];
		// Wait for sender to signal
		*flag = 0;
		while (*flag != 1) {
    		// loop until signal received
		}
		// Warm up TLB and page table for buf[0]'s page
		volatile char warmup = buf[64];
		(void)warmup;

		// Measure latency
		latency = measure_one_block_access_time((uint64_t)buf);

		if (latency > 150) { // miss, L3 cache -->  bit is 0
			bit = 0;
		} else { // hit, L2 cache --> bit is 1
			bit = 1;
		}
		listening = false;
		
	}

	printf("\nBit: %d\n", bit);
	printf("Latency: %d\n", (int)latency);
	printf("Receiver finished.\n");

	return 0;
}



