#include "utility.h"
#include <stddef.h>
#include <stdint.h>

// TODO: Uncomment the following lines and fill in the correct size
#define L1_SIZE 32768
#define L2_SIZE 1048576
#define L3_SIZE 11534336
 
int main (int ac, char **av) {

    // create 4 arrays to store the latency numbers
    // the arrays are initialized to 0
    uint64_t dram_latency[SAMPLES] = {0};
    uint64_t l1_latency[SAMPLES] = {0};
    uint64_t l2_latency[SAMPLES] = {0};
    uint64_t l3_latency[SAMPLES] = {0};

    // A temporary variable we can use to load addresses
    // The volatile keyword tells the compiler to not put this variable into a
    // register- it should always try to load from memory/ cache.
    volatile char tmp;

    // Allocate a buffer of 64 Bytes
    // the size of an unsigned integer (uint64_t) is 8 Bytes
    // Therefore, we request 8 * 8 Bytes
    uint64_t *target_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));

    if (NULL == target_buffer) {
        perror("Unable to malloc");
        return EXIT_FAILURE;
    }

    // [1.2] TODO: Uncomment the following line to allocate a buffer of a size
    // of your chosing. This will help you measure the latencies at L2 and L3.
    uint64_t *eviction_buffer = (uint64_t *)malloc(L3_SIZE + 2000000);

    if (eviction_buffer == NULL) {
        perror("Unable to malloc eviction buffer");
        return EXIT_FAILURE;
    }

    // Example: Measure L1 access latency, store results in l1_latency array
    for (int i=0; i<SAMPLES; i++){
        // Step 1: bring the target cache line into L1 by simply accessing the line
        tmp = target_buffer[0];

        // Step 2: measure the access latency
        l1_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.2] TODO: Measure DRAM Latency, store results in dram_latency array
    // ======
    //
    for (int i = 0; i < SAMPLES; i++) {
        clflush(target_buffer);
        dram_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.2] TODO: Measure L2 Latency, store results in l2_latency array
    // ======
    //
    for (int i = 0; i < SAMPLES; i++) {
        tmp = target_buffer[0];
        for (size_t j = 0; j < (L1_SIZE + 16000); j++) {
            tmp = eviction_buffer[j];
        }
        l2_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }
        

    // ======
    // [1.2] TODO: Measure L3 Latency, store results in l3_latency array
    // ======
    //
    for (int i = 0; i < SAMPLES; i++) {
        tmp = target_buffer[0];
        for (size_t j = 0; j < (L1_SIZE + L2_SIZE + 32000); j++) {
            tmp = eviction_buffer[j];
        }
        l3_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // Print the results to the screen
    // [1.5] Change print_results to print_results_for_python so that your code will work
    // with the python plotter software
    print_results(dram_latency, l1_latency, l2_latency, l3_latency);

    free(target_buffer);

    // [1.2] TODO: Uncomment this line once you uncomment the eviction_buffer creation line
    free(eviction_buffer);
    return 0;
}

