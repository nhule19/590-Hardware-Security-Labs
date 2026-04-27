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
<<<<<<< HEAD
 *
 * Inputs: mem_map - Base pointer to the large allocated pool
 *         PPN_VPN_map - Reference to a PPN->VPN map 
 *
 * Side-Effects: For *each page* in the allocated pool, the virtual page 
 *               number is into the map with a key corresponding to the 
 *               page's physical page number.
 *
=======
>>>>>>> cbb32e9 (Add part3 and shared.cc)
 */
void setup_PPN_VPN_map(void * mem_map,
                       std::map<uint64_t, uint64_t> &PPN_VPN_map) {
    // TODO: Exercise 1-3
    PPN_VPN_map.clear();

<<<<<<< HEAD
    const uint64_t total_size = 2ULL * 1024ULL * 1024ULL * 1024ULL;
    const uint64_t page_size = HUGE_PAGE_SIZE;

    for (uint64_t offset = 0; offset < total_size; offset += page_size) {
        uint64_t virt_addr = (uint64_t)mem_map + offset;
        uint64_t phys_addr = virt_to_phys(virt_addr);
        if (phys_addr == 0) {
            continue;
        }

        uint64_t ppn = phys_addr / page_size;
        uint64_t vpn = virt_addr / page_size;
        PPN_VPN_map[ppn] = vpn;
    }
    
=======
    uint64_t base = reinterpret_cast<uint64_t>(mem_map);
    uint64_t total_size = BUFFER_SIZE_MB * 1024ULL * 1024ULL;

    for (uint64_t offset = 0; offset < total_size; offset += HUGE_PAGE_SIZE) {
        uint64_t virt_addr = base + offset;
        uint64_t phys_addr = virt_to_phys(virt_addr);

        if (phys_addr != 0) {
            uint64_t phys_page_number = phys_addr >> 21;
            uint64_t virt_page_number = virt_addr >> 21;
            PPN_VPN_map[phys_page_number] = virt_page_number;
        }
    }
>>>>>>> cbb32e9 (Add part3 and shared.cc)
}

/*
 * allocate_pages
<<<<<<< HEAD
 *
 * Allocates a memory block of size BUFFER_SIZE_MB
 *
 * Make sure to write something to each page in the block to ensure
 * that the memory has actually been allocated!
 *
 * Inputs: none
 * Outputs: A pointer to the beginning of the allocated memory block
=======
>>>>>>> cbb32e9 (Add part3 and shared.cc)
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
<<<<<<< HEAD
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
=======
 */
uint64_t virt_to_phys(uint64_t virt_addr) {
>>>>>>> cbb32e9 (Add part3 and shared.cc)
    uint64_t phys_addr = 0;

    FILE * pagemap;
    uint64_t entry;

<<<<<<< HEAD
    // TODO: Exercise 1-1
    // Compute the virtual page number from the virtual address
    uint64_t virt_page_number = virt_addr / 0x1000;
    uint64_t file_offset = virt_page_number * sizeof(uint64_t);
    uint64_t page_offset = virt_addr & 0xFFF;
=======
    uint64_t vpn = virt_addr >> 12;
    uint64_t offset = virt_addr & 0xFFF;
    uint64_t file_offset = vpn * sizeof(uint64_t);
>>>>>>> cbb32e9 (Add part3 and shared.cc)

    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), file_offset, SEEK_SET) == file_offset) {
            if (fread(&entry, sizeof(uint64_t), 1, pagemap)) {
                if (entry & (1ULL << 63)) {
<<<<<<< HEAD
                    uint64_t phys_page_number = entry & ((1ULL << 54) - 1);
                    // TODO: Exercise 1-1
                    // Using the extracted physical page number,
                    // derive the physical address
                    phys_addr = (phys_page_number << 12) | page_offset;
=======
                    uint64_t ppn = entry & ((1ULL << 55) - 1);
                    phys_addr = (ppn << 12) | offset;
>>>>>>> cbb32e9 (Add part3 and shared.cc)
                } 
            }
        }
        fclose(pagemap);
    }
    return phys_addr;
}

/*
 * phys_to_virt
<<<<<<< HEAD
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
    const uint64_t page_size = HUGE_PAGE_SIZE;
    uint64_t ppn = phys_addr / page_size;
    uint64_t page_offset = phys_addr & (page_size - 1);

    auto it = PPN_VPN_map.find(ppn);
    if (it == PPN_VPN_map.end()) {
        return 0;
    }

    uint64_t vpn = it->second;
    uint64_t virt_addr = (vpn * page_size) + page_offset;
    return virt_addr;
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

=======
 */
uint64_t phys_to_virt(uint64_t phys_addr) {
    uint64_t ppn = phys_addr >> 21;
    uint64_t offset = phys_addr & (HUGE_PAGE_SIZE - 1);

    auto it = PPN_VPN_map.find(ppn);
    if (it == PPN_VPN_map.end()) return 0;

    uint64_t vpn = it->second;
    return (vpn << 21) | offset;
}

/*
 * get_rand_addr
 */
>>>>>>> cbb32e9 (Add part3 and shared.cc)
char* get_rand_addr(size_t buf_size)
{
    size_t num_cls = buf_size / CACHELINE_SIZE;
    size_t idx = rand64() % num_cls;
    return (char*)allocated_mem + idx * CACHELINE_SIZE;
}

/*
 * measure_bank_latency
<<<<<<< HEAD
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
    clflush(addr_A);
    clflush(addr_B);

    uint64_t start = get_time();
    volatile char dummy = addr_A[0];
    dummy += addr_B[0];
    uint64_t end = get_time();

    return end - start;
=======
 */
uint64_t measure_bank_latency(volatile char *addr_A, volatile char *addr_B) {
    uint64_t start, end;

    clflush(addr_A);
    clflush(addr_B);
    mfence();

    start = rdtscp64();

    *addr_A;
    *addr_B;

    end = rdtscp64();
    mfence();

    return end - start; 
>>>>>>> cbb32e9 (Add part3 and shared.cc)
}

/*
 * phys_to_bankid
<<<<<<< HEAD
 *
 * Computes the bank id of a physical address
 *
 * Inputs: phys_ptr: a physical address; candidate: the bank function derived from part3
 * Output: bank index
 *
 */

=======
 */
>>>>>>> cbb32e9 (Add part3 and shared.cc)
uint64_t phys_to_bankid(uint64_t phys_ptr, uint8_t candidate)
{
    static std::array<std::function<uint64_t(uint64_t)>, 3> functions = {

<<<<<<< HEAD
        // candidate 0
=======
>>>>>>> cbb32e9 (Add part3 and shared.cc)
        [](uint64_t x) {
            return ((get_bit(x, 14) ^ get_bit(x, 17)) << 3) | 
                   ((get_bit(x, 15) ^ get_bit(x, 18)) << 2) | 
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^
                     get_bit(x, 12) ^ get_bit(x, 13) ^
                     get_bit(x, 15) ^ get_bit(x, 16)));
        },

<<<<<<< HEAD
        // candidate 1
=======
>>>>>>> cbb32e9 (Add part3 and shared.cc)
        [](uint64_t x) {
            return ((get_bit(x, 15) ^ get_bit(x, 18)) << 3) | 
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 2) | 
                   ((get_bit(x, 17) ^ get_bit(x, 20)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^
                     get_bit(x, 12) ^ get_bit(x, 13) ^
                     get_bit(x, 18) ^ get_bit(x, 19)));
        },

<<<<<<< HEAD
        // candidate 2
=======
>>>>>>> cbb32e9 (Add part3 and shared.cc)
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
<<<<<<< HEAD
 *
 * Computes the row id of a physical address
 *
 * Inputs: phys_ptr: a physical address;
 * Output: row index
 *
 */

=======
 */
>>>>>>> cbb32e9 (Add part3 and shared.cc)
uint64_t phys_to_rowid(uint64_t phys_ptr){
	return (phys_ptr & ROW_MASK) >> __builtin_ctzl(ROW_MASK);
}

/*
 * phys_to_colid
<<<<<<< HEAD
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

=======
 */
uint64_t phys_to_colid(uint64_t phys_ptr){
    return (phys_ptr & COL_MASK) >> __builtin_ctzl(COL_MASK);
}
>>>>>>> cbb32e9 (Add part3 and shared.cc)
