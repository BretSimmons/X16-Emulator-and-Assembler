#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bits.h"



// Get the nth bit
uint16_t getbit(uint16_t number, int n) {
    return (number >> n) & 1;
}

// Get bits that are the given number of bits wide
uint16_t getbits(uint16_t number, int n, int wide) {
    // Shift target bits to the rightmost place
    number = (number >> n);
    // Mask all other bits in number
    return (number & (1U << wide) - 1);
}

// Set the nth bit to the given bit value and return the result
uint16_t setbit(uint16_t number, int n) {
    return (number |= (1U << n));
}

// Clear the nth bit
uint16_t clearbit(uint16_t number, int n) {
    return (number &= ~(1U << n));
}

// Sign extend a number of the given bits to 16 bits
uint16_t sign_extend(uint16_t x, int bit_count) {
    uint16_t mask = 0xFFFF;
    // Check if the most significant bit is 1
    if (((x >> (bit_count - 1)) & 1) == 1){
        return ((mask << bit_count) | x);
    }
    return x;
}

bool is_positive(uint16_t number) {
    return getbit(number, 15) == 0;
}

bool is_negative(uint16_t number) {
    return getbit(number, 15) == 1;
}
