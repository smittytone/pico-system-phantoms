/*
 * Phantom Slayer
 * Phantom class code
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _PHANTOM_HEADER_
#define _PHANTOM_HEADER_


/*
 * CONSTANTS
 */
constexpr uint8_t NOT_ON_BOARD = 99;

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
 * ENUMERATIONS
 */
enum class PHANTOM_DIRECTION: uint8_t {
    NONE =              0,
    NORTH =             1,
    EAST =              2,
    SOUTH =             4,
    WEST =              8,
    ERROR_CONDITION =   99
};


/*
 * PROTOTYPES
 */
class Phantom {
    public:
        // Constructor
        Phantom();
        // Methods
        void                init(void);
        void                place(uint8_t my_index);
        bool                move(void);
        void                move_one_square(uint8_t nd, uint8_t* nx, uint8_t* ny) const;
        PHANTOM_DIRECTION   came_from(void) const;
        // Properties
        uint8_t             x;
        uint8_t             y;
        int8_t              hp;
        PHANTOM_DIRECTION   direction;
        uint8_t             back_steps;
};


#endif  // _PHANTOM_HEADER_
