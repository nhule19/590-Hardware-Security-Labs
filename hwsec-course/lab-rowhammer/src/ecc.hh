#ifndef ECC_HH
#define ECC_HH

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <vector>

#define NUM_DATA_BITS 16
#define NUM_PARITY_BITS 6

#define TOTAL_BITS (NUM_DATA_BITS + NUM_PARITY_BITS)

enum _ERROR_TYPE {NO_ERROR, SINGLE_ERROR, DOUBLE_ERROR, PARITY_ERROR};

// Used to store both the data and parity bits separately
struct hamming_struct{
    uint32_t data;
    uint32_t parity;
};

struct hamming_result{
    _ERROR_TYPE error;
    uint32_t syndrome;
};


// Gets the bit of 'data' located at position 'pos'
inline uint32_t getBit(uint32_t data, uint8_t pos) {
    return (data >> pos) & 1;
}

// Flips the bit of 'data' located at position 'pos'
inline uint32_t flipBit(uint32_t data, uint8_t pos) {
    if ((data >> pos) & 1) {
        return data & ~(1 << pos);
    } else {
        return data | 1 << pos;
    }
}

// Returns true if bitNum corresponds to a parity bit 
inline bool isParityBit(uint32_t bitNum) {
    return (((bitNum+1) & (bitNum)) == 0) | (bitNum == 21);
}


// Decodes an ECC-protected value into its corresponding data and parity bits
inline hamming_struct extractEncoding(uint32_t encoded) {
    uint32_t parity = 0;
    uint32_t data = 0;

    uint32_t paritySeen = 0;
    for (int i = 0; i<TOTAL_BITS; i++) {
        if (isParityBit(i)) {
            parity |= getBit(encoded, i) << paritySeen;
            paritySeen++;
        } else {
            data |= getBit(encoded, i) << (i - paritySeen);
        }
    }
    
    return {data, parity};
}

// Encodes separate data and parity bits into a single ECC-protected value
inline uint32_t embedEncoding(struct hamming_struct hamming) {
    uint32_t encoded = 0;
    uint32_t paritySeen = 0;

    for (int i = 0; i<TOTAL_BITS; i++) {
        if (isParityBit(i)) {
            encoded |= getBit(hamming.parity,paritySeen) << i;
            paritySeen++;
        } else {
            encoded |= getBit(hamming.data,i-paritySeen) << i;
        }
    }
    
    return encoded;
}

// Injects "numFlips" number of faults into an ECC-protected value "data"
inline uint32_t injectRandomFlips(uint32_t data, uint8_t numFlips) {
    uint32_t faulty = data;
    std::vector<uint8_t> indices;

    for (int i = 0; i < TOTAL_BITS-1; i++) {
        indices.push_back(i);
    }

    std::random_shuffle ( indices.begin(), indices.end() );

    for (uint8_t i = 0; i < numFlips; i++) {
        faulty = flipBit(faulty,indices.back());
        indices.pop_back();
    }

    return faulty; 
}

#endif
