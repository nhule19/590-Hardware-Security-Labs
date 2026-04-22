#include "../shared.hh"
#include "../verif.hh"
#include "../params.hh"

/*
 *
 * DO NOT MODIFY BELOW ME
 *
 */

#define NUM_ITERATIONS 100

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);

    // Allocate a large pool of memory (of size BUFFER_SIZE_MB)
    // pointed to by allocated_mem
    allocated_mem = allocate_pages(BUFFER_SIZE_MB * 1024UL * 1024UL);

    verify_virt_to_phys(allocated_mem, &virt_to_phys);

    // Setup, then verify the PPN_VPN_map
    setup_PPN_VPN_map(allocated_mem, PPN_VPN_map);
    verify_PPN_VPN_map(allocated_mem, PPN_VPN_map);
    
    verify_phys_to_virt(allocated_mem, &phys_to_virt);

}
