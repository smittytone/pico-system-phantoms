/*
 * Phantom Slayer
 * Utility functions
 *
 * @version     1.1.3
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using namespace picosystem;

static inline uint8_t rnd_hw(void);

/*
 *      EXTERNALLY-DEFINED GLOBALS
 */
extern tinymt32_t   tinymt_store;
uint8_t rnd_last = rnd_hw();

/*
 *      GLOBALS
 */
// NOTE Key values defined by picosystem
uint8_t keys[8] = {A, B, X, Y, UP, DOWN, LEFT, RIGHT};


namespace Utils {

/**
    Randomise using TinyMT
    https://github.com/MersenneTwister-Lab/TinyMT
    Generate a PRG between 0 and max-1 then add start,
    eg. 10, 20 -> range 10-29

    - Parameters:
        - start: A baseline value added to the rolled value.
        - max:   A maximum roll.

    - Returns: The random number.
 */
int irandom(int start, int max) {

    //int value = tinymt32_generate_uint32(&tinymt_store);
    //return ((value % max) + start);

    int value = rnd_hw() << rnd_last;
    rnd_last = rnd_hw();
    return ((value % max) + start);
}


/**
    Check all the keys to see if any have been pressed. Set a
    bit for each key set in the order:
    A (Bit 0), B, X, Y, UP, DOWN, LEFT, RIGHT (Bit 7)
 */
uint8_t inkey() {

    uint8_t bits = 0;
    for (uint8_t i = 0 ; i < 8 ; ++i) {
        if (pressed(keys[i])) bits |= (1 << i);
    }

    return bits;
}


/**
    Convert a 16-bit int (to cover decimal range 0-9999) to
    its BCD equivalent.

    - Parameters:
        - base: The input integer.

    - Returns: The BCD encoding of the input.
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


static inline uint8_t rnd_hw(void) {
    uint8_t rnd = 0;
    for (int i = 0; i < 8; i++)
        rnd |= ((rosc_hw->randombit & 1) << i);
    return (rnd);
}
