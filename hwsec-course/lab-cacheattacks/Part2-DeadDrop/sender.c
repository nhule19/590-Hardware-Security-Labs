
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

// TODO: define your own buffer size
#define BUFF_SIZE (1<<21)

#define L1_SIZE 32768
#define L2_SIZE 1048576

#define LINE_SIZE 64

int main(int argc, char **argv)
{
    // Allocate a buffer using huge page
    // See the handout for details about hugepage management
    void *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
  
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

    // TODO:
    // Put your covert channel setup code here

    uint64_t *eviction_buffer = (uint64_t *)malloc(L1_SIZE + L2_SIZE + 10000);

    if (eviction_buffer == NULL) {
        perror("Unable to malloc eviction buffer");
        return EXIT_FAILURE;
    }

    printf("Please type a message.\n");

    bool sending = true;
    while (sending) {
        char text_buf[128];
        fgets(text_buf, sizeof(text_buf), stdin);

        // TODO:
        // Put your covert channel code here

        char c = text_buf[0];
        int transmit_bit = c & 0b1;

        if (transmit_bit == 0b1) { // make latency small
            for (int i = 0; i < 1090; i++) {
                tmp = ((char*)buf)[i * 64];
            }
        } else { // make latency big (evict)
            for (int i = 0; i < 1090; i++) {
                tmp = eviction_buffer[i];
            }
        }
        sending = false;
    }

  printf("Sender finished.\n");
  return 0;
}


