#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bits.h"



// Get the nth bit
uint16_t getbit(uint16_t number, int n) {
    // TODO: Your code here
    return 0;
}

// Get bits that are the given number of bits wide
uint16_t getbits(uint16_t number, int n, int wide) {
    // TODO: Your code here
    return 0;
}

// Set the nth bit to the given bit value and return the result
uint16_t setbit(uint16_t number, int n) {
    // TODO: Your code here
    return 0;
}

// Clear the nth bit
uint16_t clearbit(uint16_t number, int n) {
    // TODO: Your code here
    return 0;
}

// Sign extend a number of the given bits to 16 bits
uint16_t sign_extend(uint16_t x, int bit_count) {
    // TODO: Your code here
    return 0;
}

bool is_positive(uint16_t number) {
    return getbit(number, 15) == 0;
}

bool is_negative(uint16_t number) {
    return getbit(number, 15) == 1;
}
