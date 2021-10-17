/*
 * Phantom Slayer
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _PHANTOMS_HEADER_
#define _PHANTOMS_HEADER_


/*
 * PROTOTYPES
 */
class Phantom {
    public:
        Phantom();

        void        move();
        void        move_one(uint8_t c, uint8_t *x, uint8_t *y);
        uint8_t     from_direction();
        uint8_t     locate_phantom(uint8_t x, uint8_t y);
    
        void        manage_phantoms();
        void        roll_new_phantom(uint8_t index);
        uint8_t     get_facing_phantom(uint8_t range);
        uint8_t     count_facing_phantoms(uint8_t range);

        uint8_t     x;
        uint8_t     y;
        uint8_t     hp;
        uint8_t     hp_max;
        uint8_t     direction;
        uint8_t     back_steps;
}



#endif  // _PHANTOMS_HEADER_
