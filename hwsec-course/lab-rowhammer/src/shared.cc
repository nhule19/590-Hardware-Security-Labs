#include <emmintrin.h>

#include "shared.hh"
#include "params.hh"
#include "util.hh"

// Physical Page Number to Virtual Page Number Map
std::map<uint64_t, uint64_t> PPN_VPN_map;

// Base pointer to a large memory pool
void * allocated_mem;

/*
 * setup_PPN_VPN_map
 *
 * Populates the Physical Page Number -> Virtual Page Number mapping table
 *
 * Inputs: mem_map - Base pointer to the large allocated pool
 *         PPN_VPN_map - Reference to a PPN->VPN map 
 *
 * Side-Effects: For *each page* in the allocated pool, the virtual page 
 *               number is into the map with a key corresponding to the 
 *               page's physical page number.
 *
 */
void setup_PPN_VPN_map(void * mem_map,
                       std::map<uint64_t, uint64_t> &PPN_VPN_map) {
    // TODO: Exercise 1-3
}

/*
 * allocate_pages
 *
 * Allocates a memory block of size BUFFER_SIZE_MB
 *
 * Make sure to write something to each page in the block to ensure
 * that the memory has actually been allocated!
 *
 * Inputs: none
 * Outputs: A pointer to the beginning of the allocated memory block
 */
void * allocate_pages(uint64_t memory_size) {
    void * memory_block = mmap(NULL, memory_size, PROT_READ | PROT_WRITE,
            MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    assert(memory_block != (void*)-1);

    for (uint64_t i = 0; i < memory_size; i += HUGE_PAGE_SIZE) {
        uint64_t * addr = (uint64_t *) ((uint8_t *) (memory_block) + i);
        *addr = i;
    } 

    return memory_block;
}

/* 
 * virt_to_phys
 *
 * Determines the physical address mapped to by a given virtual address
 *
 * Inputs: virt_addr - A virtual pointer/address
 * Output: phys_ptr - The physical address corresponding to the virtual pointer
 *                    IMPORTANT: If the virtual pointer is not currently
 *                               present, return 0
 *
 */


uint64_t virt_to_phys(uint64_t virt_addr) {
    uint64_t phys_addr = 0;

    FILE * pagemap;
    uint64_t entry;

    // TODO: Exercise 1-1
    // Compute the virtual page number from the virtual address
    uint64_t virt_page_number = virt_addr / 0x1000;
    uint64_t file_offset = virt_page_number * sizeof(uint64_t);

    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), file_offset, SEEK_SET) == file_offset) {
            if (fread(&entry, sizeof(uint64_t), 1, pagemap)) {
                if (entry & (1ULL << 63)) {
                    uint64_t phys_page_number = entry & ((1ULL << 54) - 1);
                    // TODO: Exercise 1-1
                    // Using the extracted physical page number,
                    // derive the physical address
                    phys_addr = 0;
                } 
            }
        }
        fclose(pagemap);
    }
    return phys_addr;
}

/*
 * phys_to_virt
 *
 * Determines the virtual address mapping to a given physical address
 *
 * HINT: This should use your PPN_VPN_map!
 *
 * Inputs: phys_addr - A physical pointer/address
 * Output: virt_addr - The virtual address corresponding to the physical pointer
 *                     If the physical pointer is not mapped, return 0
 *
 */

uint64_t phys_to_virt(uint64_t phys_addr) {
    // TODO: Exercise 1-4
    return 0;
}


/*
 * get_rand_addr
 *
 * Gets a random virtual address (aligned to cacheline size)
 *
 *
 * Inputs: allocate buffer size 
 * Output: virt_addr - A random virtual address within the allocated memory
 *
 */

char* get_rand_addr(size_t buf_size)
{
    size_t num_cls = buf_size / CACHELINE_SIZE;
    size_t idx = rand64() % num_cls;
    return (char*)allocated_mem + idx * CACHELINE_SIZE;
}

/*
 * measure_bank_latency
 *
 * Measures a (potential) bank collision between two addresses,
 * and returns its timing characteristics.
 *
 * Inputs: addr_A/addr_B - Two (virtual) addresses used to observe
 *                         potential contention
 * Output: Timing difference (derived by a scheme of your choice)
 *
 */
uint64_t measure_bank_latency(volatile char *addr_A, volatile char *addr_B) {
    // TODO: Exercise 2-2
    return 0; 
}

/*
 * phys_to_bankid
 *
 * Computes the bank id of a physical address
 *
 * Inputs: phys_ptr: a physical address; candidate: the bank function derived from part3
 * Output: bank index
 *
 */

uint64_t phys_to_bankid(uint64_t phys_ptr, uint8_t candidate)
{
    static std::array<std::function<uint64_t(uint64_t)>, 3> functions = {

        // candidate 0
        [](uint64_t x) {
            return ((get_bit(x, 14) ^ get_bit(x, 17)) << 3) | 
                   ((get_bit(x, 15) ^ get_bit(x, 18)) << 2) | 
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^
                     get_bit(x, 12) ^ get_bit(x, 13) ^
                     get_bit(x, 15) ^ get_bit(x, 16)));
        },

        // candidate 1
        [](uint64_t x) {
            return ((get_bit(x, 15) ^ get_bit(x, 18)) << 3) | 
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 2) | 
                   ((get_bit(x, 17) ^ get_bit(x, 20)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^
                     get_bit(x, 12) ^ get_bit(x, 13) ^
                     get_bit(x, 18) ^ get_bit(x, 19)));
        },

        // candidate 2
        [](uint64_t x) {
            return ((get_bit(x, 13) ^ get_bit(x, 17)) << 3) | 
                   ((get_bit(x, 14) ^ get_bit(x, 18)) << 2) | 
                   ((get_bit(x, 15) ^ get_bit(x, 19)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^
                     get_bit(x, 12) ^ get_bit(x, 13) ^
                     get_bit(x, 20) ^ get_bit(x, 21)));
        },
    };

    return functions[candidate](phys_ptr) & 0xF;
}

/*
 * phys_to_rowid
 *
 * Computes the row id of a physical address
 *
 * Inputs: phys_ptr: a physical address;
 * Output: row index
 *
 */

uint64_t phys_to_rowid(uint64_t phys_ptr){
	return (phys_ptr & ROW_MASK) >> __builtin_ctzl(ROW_MASK);
}

/*
 * phys_to_colid
 *
 * Computes the column id of a physical address
 *
 * Inputs: phys_ptr: a physical address;
 * Output: column index
 *
 */

uint64_t phys_to_colid(uint64_t phys_ptr){
    return (phys_ptr & COL_MASK) >> __builtin_ctzl(COL_MASK);
}

