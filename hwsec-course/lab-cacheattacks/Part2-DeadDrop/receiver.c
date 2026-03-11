
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <stdint.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h> 

#define BUFF_SIZE (1<<21)


void delay (int seconds) {
	long pause = seconds * CLOCKS_PER_SEC;
	clock_t start = clock();
	while (clock() - start < pause);
}

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

	printf("Receiver now listening.\n");

	bool listening = true;
	int bit;
	int latency;

	while (listening) {
		delay(4);
		// Put your covert channel code here
		int total = 0;
		for (int i = 0; i < 100; i++) {
			total += measure_one_block_access_time((uint64_t)buf);
		}
		latency = total / 100;

		if (latency > 110) { // miss, L3 cache -->  bit is 0
			bit = 0;
		} else { // hit, L2 cache --> bit is 1
			bit = 1;
		}
		listening = false;
	}

	printf("\nBit: %d\n", bit);
	printf("Latency: %d\n", latency);
	printf("Receiver finished.\n");

	return 0;
}



