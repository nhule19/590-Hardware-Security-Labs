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
 */
void setup_PPN_VPN_map(void * mem_map,
                       std::map<uint64_t, uint64_t> &PPN_VPN_map) {
    // TODO: Exercise 1-3
    PPN_VPN_map.clear();

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
}

/*
 * allocate_pages
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
 */
uint64_t virt_to_phys(uint64_t virt_addr) {
    uint64_t phys_addr = 0;

    FILE * pagemap;
    uint64_t entry;

    uint64_t vpn = virt_addr >> 12;
    uint64_t offset = virt_addr & 0xFFF;
    uint64_t file_offset = vpn * sizeof(uint64_t);

    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), file_offset, SEEK_SET) == file_offset) {
            if (fread(&entry, sizeof(uint64_t), 1, pagemap)) {
                if (entry & (1ULL << 63)) {
                    uint64_t ppn = entry & ((1ULL << 55) - 1);
                    phys_addr = (ppn << 12) | offset;
                } 
            }
        }
        fclose(pagemap);
    }
    return phys_addr;
}

/*
 * phys_to_virt
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
char* get_rand_addr(size_t buf_size)
{
    size_t num_cls = buf_size / CACHELINE_SIZE;
    size_t idx = rand64() % num_cls;
    return (char*)allocated_mem + idx * CACHELINE_SIZE;
}

/*
 * measure_bank_latency
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
}

/*
 * phys_to_bankid
 */
uint64_t phys_to_bankid(uint64_t phys_ptr, uint8_t candidate)
{
    static std::array<std::function<uint64_t(uint64_t)>, 3> functions = {

        [](uint64_t x) {
            return ((get_bit(x, 14) ^ get_bit(x, 17)) << 3) | 
                   ((get_bit(x, 15) ^ get_bit(x, 18)) << 2) | 
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^
                     get_bit(x, 12) ^ get_bit(x, 13) ^
                     get_bit(x, 15) ^ get_bit(x, 16)));
        },

        [](uint64_t x) {
            return ((get_bit(x, 15) ^ get_bit(x, 18)) << 3) | 
                   ((get_bit(x, 16) ^ get_bit(x, 19)) << 2) | 
                   ((get_bit(x, 17) ^ get_bit(x, 20)) << 1) |
                   ((get_bit(x, 7) ^ get_bit(x, 8) ^ get_bit(x, 9) ^
                     get_bit(x, 12) ^ get_bit(x, 13) ^
                     get_bit(x, 18) ^ get_bit(x, 19)));
        },

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
 */
uint64_t phys_to_rowid(uint64_t phys_ptr){
	return (phys_ptr & ROW_MASK) >> __builtin_ctzl(ROW_MASK);
}

/*
 * phys_to_colid
 */
uint64_t phys_to_colid(uint64_t phys_ptr){
    return (phys_ptr & COL_MASK) >> __builtin_ctzl(COL_MASK);
}