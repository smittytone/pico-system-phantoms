/*
 * Phantom Slayer
 * Map management functions
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _PHANTOMS_MAP_HEADER_
#define _PHANTOMS_MAP_HEADER_


/*
 * CONSTANTS
 */
#define NUMBER_OF_MAPS              6
#define MAP_MAX                     19


/*
 * PROTOTYPES
 */
namespace Map {
    uint8_t         init(uint8_t last_map) ;
    void            draw(uint8_t y_delta, bool show_entities, bool show_tele = true);
    bool            set_square_contents(uint8_t x, uint8_t y, uint8_t value);
    uint8_t         get_square_contents(uint8_t x, uint8_t y);
    uint8_t         get_view_distance(int8_t x, int8_t y, uint8_t direction);
    uint8_t         phantom_on_square(uint8_t x, uint8_t y);
}


// _PHANTOMS_MAP_HEADER_
#endif
