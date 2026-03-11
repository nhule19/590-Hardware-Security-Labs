
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <stdint.h>
#include <sys/mman.h>
#define BUFF_SIZE (1<<21)
#define TIMESLOT 100000
#define THRESHOLD 200

int main(int argc, char **argv)
{
	// Put your covert channel setup code here
	void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);

	if (buf == (void*) - 1) {
		perror("mmap() error\n");
		exit(EXIT_FAILURE);
	}

	*((char*)buf) = 1;
    volatile char tmp;

	printf("Please press enter.\n");

	char text_buf[2];
	fgets(text_buf, sizeof(text_buf), stdin);

	printf("Receiver now listening.\n");

	bool listening = true;
	int current_char = 0;
	int bit_count = 0;
	while (listening) {

		// Put your covert channel code here

		uint64_t latency = measure_one_block_access_time((uint64_t)buf);

		for (size_t j = 64; j < BUFF_SIZE; j += 64) {
    		tmp = ((char*)buf)[j];
		}
		
        int bit;

        if (latency > THRESHOLD) {
			bit = 1;
		} else {
			bit = 0;
		}
    
        current_char = (current_char << 1) | bit;
        bit_count++;

        if (bit_count == 8) {
            printf("%c", current_char);
            fflush(stdout);
            bit_count = 0;
            current_char = 0;
        }
		for (volatile int i = 0; i < TIMESLOT; i++);
    }
	printf("Receiver finished.\n");

	return 0;
}


