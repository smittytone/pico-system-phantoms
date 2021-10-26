/*
 * Phantom Slayer
 * Utility functions
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _GAME_UTILS_HEADER_
#define _GAME_UTILS_HEADER_


/*
 * PROTOYPES
 */
namespace Utils {
    int             irandom(int start, int max);
    uint8_t         inkey();
    uint32_t        bcd(uint32_t base);
}


#endif  // _GAME_UTILS_HEADER_
