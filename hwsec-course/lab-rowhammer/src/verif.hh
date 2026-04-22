#ifndef VERIF_VERIF_HH
#define VERIF_VERIF_HH

#include <cstdint>
#include <inttypes.h>
#include <map>

#include "params.hh"
#include "ecc.hh"


// part 1 checker
void verify_phys_to_virt(void *mem_map, uint64_t (*fun)(uint64_t));
void verify_virt_to_phys(void *mem_map, uint64_t (*fun)(uint64_t));
void verify_PPN_VPN_map(void *mem_map, std::map<uint64_t, uint64_t> &PPN_VPN_map);

// part 5 checker
int checkParity(uint32_t encoded);

#endif
