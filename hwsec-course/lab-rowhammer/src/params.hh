#ifndef PARAMS_HH
#define PARAMS_HH

// Size of allocated buffer
#define BUFFER_SIZE_MB 1024

// Size of hugepages in system
#define HUGE_PAGE_SIZE (1ULL<< 21)
#define HUGE_PAGE_MASK  (~(HUGE_PAGE_SIZE - 1))

// Size of DRAM row (1 bank)
#define ROW_SIZE (8192)

#define CACHELINE_SIZE 64

#define ROW_STRIDE   (1ULL<<17)   

#define COL_MASK 0x1fff

#define ROW_MASK 0xfffe0000

// Number of hammers to perform per iteration
#define HAMMERS_PER_ITER 5000000

#endif
