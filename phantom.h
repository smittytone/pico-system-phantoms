/*
 * Phantom Slayer
 * Phantom class code
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _PHANTOM_HEADER_
#define _PHANTOM_HEADER_


/*
 *  CONSTANTS
 */
# define NOT_ON_BOARD       99


const uint8_t level_data[84] = {
    1,1,1,0,        // 1
    1,2,1,0,        // 2
    1,3,1,0,        // 3
    1,4,0,0,        // 4
    1,5,0,0,        // 5
    1,6,0,0,        // 6
    2,6,0,0,        // 7
    2,6,0,0,        // 8
    2,6,0,0,        // 9
    3,6,0,1,        // 10
    3,6,0,1,        // 11
    3,6,0,1,        // 12
    4,6,0,1,        // 13
    4,6,0,1,        // 14
    4,6,0,1,        // 15
    4,8,0,2,        // 16
    4,8,0,2,        // 17
    4,8,0,2,        // 18
    5,9,0,2,        // 19
    5,9,0,2,        // 20
    5,9,0,2         // 21
};


/*
 *  PROTOTYPES
 */
class Phantom {
    public:
        // Methods
        Phantom(uint8_t start_x = NOT_ON_BOARD, uint8_t start_y = NOT_ON_BOARD);

        void        move();
        void        move_one_square(uint8_t nd, uint8_t* nx, uint8_t* ny);
        uint8_t     came_from();
        void        roll_location();
        uint8_t     getx();
        void        setx(uint8_t v);


        // Properties
        uint8_t     x;
        uint8_t     y;
        uint8_t     hp;
        uint8_t     hp_max;
        uint8_t     direction;
        uint8_t     back_steps;
};


#endif  // _PHANTOM_HEADER_
