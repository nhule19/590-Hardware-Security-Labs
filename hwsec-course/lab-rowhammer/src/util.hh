#ifndef UTIL_HH
#define UTIL_HH

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sched.h>



// Function to read the time stamp counter, which is called tsc for short
// "rdtscp" returns a 32bit unsigned integer
// "rdtscp64" return a 64 bit unsigned integer
// Details in https://www.felixcloutier.com/x86/rdtscp
static inline uint32_t rdtscp() {
    uint32_t rv;
    asm volatile ("rdtscp": "=a" (rv) :: "edx", "ecx");
    return rv;
}

static inline uint64_t rdtscp64() {
    uint32_t low, high;
    asm volatile ("rdtscp": "=a" (low), "=d" (high) :: "ecx");
    return (((uint64_t)high) << 32) | low;
}

// Function "lfence" wrap the assembly instruction lfence
// This function performs a serializing operation which ensures that
// the instructions after "lfence" start execution after
// all the instructions before "lfence" complete
// Details in https://www.felixcloutier.com/x86/lfence
static inline void lfence() {
    asm volatile("lfence");
}

static inline __attribute__((always_inline)) void mfence() 
{
	asm volatile ("mfence" : : : "memory");
}


// Here is an example of using "rdtsc" and "lfence" to
// measure the time it takes to access a block specified by its virtual address
// The corresponding pseudo code is
// =========
// t1 = rdtsc
// load addr
// t2 = rdtsc
// cycles = t2 - t1
// return cycles
// =========
static inline uint64_t measure_one_block_access_time(uint64_t addr)
{
    uint64_t cycles;

    asm volatile("mov %1, %%r8\n\t"
    "lfence\n\t"
    "rdtsc\n\t"
    "mov %%eax, %%edi\n\t"
    "mov (%%r8), %%r8\n\t"
    "lfence\n\t"
    "rdtsc\n\t"
    "sub %%edi, %%eax\n\t"
    : "=a"(cycles) /*output*/
    : "r"(addr)    /*input*/
    : "r8", "edi"); /*reserved register*/

    return cycles;
}


static inline void one_block_access(uint64_t addr)
{
    asm volatile("mov (%0), %%r8\n\t"
    : /*output*/
    : "r"(addr)    /*input*/
    : "r8");    /*reserved register*/

}


// A wrapper function of the clflush instruction
// The instruction evict the given address from the cache to DRAM
// so that the next time the line is accessed, it will be fetched from DRAM
// Details in https://www.felixcloutier.com/x86/clflush
// The "memory" at the end of the line ask compiler not reorder memory loads
// static inline void clflush(void *v) {
//     asm volatile ("clflush (%0)": : "r" (v) : "memory");
// }

static inline __attribute__((always_inline)) void clflush(volatile void *p)
{
	asm volatile("clflush (%0)\n"
		:: "r" (p) : "memory");
}


// A rand64() function to gen a random uint64_t number
static inline uint64_t rand64()
{
    return ((uint64_t)rand() << 32) | rand();
}

inline uint64_t get_bit(uint64_t x, int bit) {
    return (x >> bit) & 1;
}

#endif

