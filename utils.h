/*
 * Phantom Slayer
 * Utility functions
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
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
    uint8_t         inkey(void);
    uint32_t        bcd(uint32_t base);
}


#endif  // _GAME_UTILS_HEADER_
