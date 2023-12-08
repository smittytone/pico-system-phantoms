/*
 * Phantom Slayer
 * Utility functions
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using namespace picosystem;


/*
 * EXTERNALLY-DEFINED GLOBALS
 */
extern tinymt32_t   tinymt_store;


/*
 * GLOBALS
 */
// NOTE Key values defined by picosystem
uint8_t keys[8] = {A, B, X, Y, UP, DOWN, LEFT, RIGHT};


namespace Utils {

/**
 * @brief Randomise using TinyMT
 *        https://github.com/MersenneTwister-Lab/TinyMT
 *        Generate a PRG between 0 and max-1 then add start,
 *        eg. 10, 20 -> range 10-29
 *
 * @param start: A baseline value added to the rolled value.
 * @param max:   A maximum roll.
 *
 * @returns: The random number.
 */
int irandom(int start, int max) {

    int value = tinymt32_generate_uint32(&tinymt_store);
    return ((value % max) + start);
}


/**
 * @brief Check all the keys to see if any have been pressed. Set a
 *        bit for each key set in the order:
 *        A (Bit 0), B, X, Y, UP, DOWN, LEFT, RIGHT (Bit 7)
 */
uint8_t inkey(void) {

    uint8_t bits = 0;
    for (uint8_t i = 0 ; i < 8 ; ++i) {
        if (pressed(keys[i])) bits |= (1 << i);
    }

    return bits;
}


/**
 * @brief Convert a 16-bit int (to cover decimal range 0-9999) to
 *        its BCD equivalent.
 * @param base: The input integer.
 *
 * @returns: The BCD encoding of the input.
 */
uint32_t bcd(uint32_t base) {

    if (base > 9999) base = 9999;
    for (uint32_t i = 0 ; i < 16 ; ++i) {
        base = base << 1;
        if (i == 15) break;
        if ((base & 0x000F0000) > 0x0004FFFF) base += 0x00030000;
        if ((base & 0x00F00000) > 0x004FFFFF) base += 0x00300000;
        if ((base & 0x0F000000) > 0x04FFFFFF) base += 0x03000000;
        if ((base & 0xF0000000) > 0x4FFFFFFF) base += 0x30000000;
    }

    return (base >> 16) & 0xFFFF;
}


}   // namespace Utils
