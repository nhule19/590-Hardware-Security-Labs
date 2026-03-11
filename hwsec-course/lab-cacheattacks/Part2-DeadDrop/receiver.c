
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <stdint.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
	// Put your covert channel setup code here

	void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
  
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
	while (listening) {

		// Put your covert channel code here
		int latency = measure_one_block_access_time((uint64_t)buf);

		if (latency > 105) { // miss, L3 cache -->  bit is 0
			bit = 0;
		} else { // hit, L2 cache --> bit is 1
			bit = 1;
		}
	}

	printf("\nBit: %d", bit);
	printf("Receiver finished.\n");

	return 0;
}

