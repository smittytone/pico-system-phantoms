/*
 * phantom-slayer for Raspberry Pi Pico
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _PHANTOM_SLAYER_MAIN_HEADER_
#define _PHANTOM_SLAYER_MAIN_HEADER_


/*
 * Includes
 */
// System, C++
#include "picosystem.hpp"
#include "hardware/adc.h"
#include "hardware/regs/rosc.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstring>
// Application
#include "gfx.h"
#include "help.h"
#include "map.h"
#include "phantom.h"
#include "tinymt32.h"
#include "utils.h"



#ifdef __cplusplus
extern "C" {
#endif


/*
 * ENUMERATIONS
 */
// Player movement directions
enum class DIRECTION: uint8_t {
    NORTH =             0,
    EAST,
    SOUTH,
    WEST
};

enum class MOVE: uint8_t {
    FORWARD =           0,
    RIGHT =             1,
    BACKWARD =          2,
    LEFT =              3,
    ERROR_CONDITION =   99
};

// Game states
enum class GAME_STATE: uint8_t {
    NOT_IN_PLAY,
    ANIMATE_LOGO,
    ANIMATE_CREDIT,
    LOGO_PAUSE,
    DO_STAB,
    OFFER_HELP,
    SHOW_HELP,
    START_COUNT,
    IN_PLAY,
    DO_TELEPORT_ONE,
    DO_TELEPORT_TWO,
    ZAP_PHANTOM,
    SHOW_TEMP_MAP,
    PLAYER_IS_DEAD,
    NEW_GAME_OFFER,
    ANIMATE_RIGHT_TURN,
    ANIMATE_LEFT_TURN
};

// Keys
enum class KEY: uint8_t {
    A               = 0x01,
    B               = 0x02,
    X               = 0x04,
    Y               = 0x08,
    UP              = 0x10,
    DOWN            = 0x20,
    LEFT            = 0x40,
    RIGHT           = 0x80
};


/*
 * CONSTANTS
 */
// Timer limits
constexpr uint32_t PHANTOM_MOVE_TIME_US     = 1000000;
constexpr uint32_t LASER_RECHARGE_US        = 2000000;
constexpr uint32_t MAP_POST_KILL_SHOW_MS    = 3000;
constexpr uint32_t LASER_FIRE_US            = 200000;
constexpr uint32_t LOGO_ANIMATION_US        = 9000;
constexpr uint32_t LOGO_PAUSE_TIME          = 5000000;
// Map square types
constexpr uint8_t MAP_TILE_WALL             = 0xEE;
constexpr uint8_t MAP_TILE_CLEAR            = 0xFF;
constexpr uint8_t MAP_TILE_TELEPORTER       = 0xAA;
// Maximum viewable distance
constexpr uint8_t MAX_VIEW_RANGE            = 5;
// ???
constexpr uint8_t BASE_MAP_Y_DELTA          = 4;
// Turn animation screen slice size
constexpr uint8_t SLICE                     = 16;
// No Phantom in view
constexpr uint8_t NONE                      = 99;
constexpr uint8_t MAX_PHANTOMS              = 5;


/*
 * STRUCTURE DEFINITIONS
 */
typedef struct {
    uint8_t                 x;
    uint8_t                 y;
    DIRECTION               direction;
} Player;

typedef struct {
    bool                    show_reticule;
    bool                    can_fire;
    bool                    is_firing;

    std::vector<Phantom>    phantoms;
    uint8_t                 phantom_count;
    uint32_t                phantom_speed;
    uint32_t                last_phantom_move;
    int8_t                  crosshair_delta;

    Player                  player;
    bool                    is_dead;

    GAME_STATE              state;
    uint8_t                 map;
    uint8_t                 audio_range;
    uint8_t                 tele_x;
    uint8_t                 tele_y;
    uint8_t                 start_x;
    uint8_t                 start_y;

    uint16_t                level;
    uint16_t                score;
    uint16_t                high_score;
    uint16_t                kills;
    uint16_t                level_kills;
    uint16_t                level_hits;

    uint32_t                zap_charge_time;
    uint32_t                zap_fire_time;
    uint8_t                 zap_frame;
} Game;

typedef struct {
    uint8_t                 x;
    uint8_t                 y;
    uint8_t                 width;
    uint8_t                 height;
    uint8_t                 spot;
} Rect;


/*
 * PROTOTYPES
 */
void                setup_device(void);
void                start_new_game(void);
void                init_game(void);
void                init_phantoms(void);
void                init_level(void);
void                start_new_level(void);
void                set_teleport_square(void);

void                update_world(void);
void                check_senses(void);
bool                move_phantoms(void);
void                manage_phantoms(void);

MOVE                get_direction(uint8_t key_pressed);
uint8_t             get_facing_phantom(uint8_t range);
uint8_t             count_facing_phantoms(uint8_t range);

void                fire_laser(void);
void                reset_laser(void);
void                do_teleport(void);

void                death(void);
void                phantom_killed(bool is_last = false);
void                show_scores(bool show_tele = false);
uint8_t             fix_num_width(uint8_t value, uint8_t current);

void                beep(void);
inline DIRECTION    do_turn_right(DIRECTION current);
inline DIRECTION    do_turn_left(DIRECTION current);

#ifdef __cplusplus
}
#endif



#endif // _PHANTOM_SLAYER_MAIN_HEADER_
