/*
 * Phantom Slayer
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _PHANTOM_HEADER_
#define _PHANTOM_HEADER_


# define NOT_ON_BOARD       99


/*
 * PROTOTYPES
 */
class Phantom {
    public:
        // Methods
        Phantom(uint8_t start_x = NOT_ON_BOARD, uint8_t start_y = NOT_ON_BOARD);

        void        move();
        void        move_one_square(uint8_t c, uint8_t *x, uint8_t *y);
        uint8_t     came_from();

        // Properties
        uint8_t     x;
        uint8_t     y;
        uint8_t     hp;
        uint8_t     hp_max;
        uint8_t     direction;
        uint8_t     back_steps;
};


#endif  // _PHANTOM_HEADER_
