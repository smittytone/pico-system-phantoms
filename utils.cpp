/*
 * Phantom Slayer
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using namespace picosystem;

uint8_t keys[8] = {A, B, X, Y, UP, DOWN, LEFT, RIGHT};

namespace Utils {

/*
    Randomise using TinyMT
    https://github.com/MersenneTwister-Lab/TinyMT
    Generate a PRG between 0 and max-1 then add start,
    eg. 10, 20 -> range 10-29

 */
int irandom(int start, int max) {
    uint32_t value = tinymt32_generate_uint32(&tinymt_store);
    return (value % max + start);
}

/*
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

}   // namespace Utils
