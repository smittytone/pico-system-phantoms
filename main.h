/*
 * phantom-slayer for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _PHANTOM_SLAYER_MAIN_HEADER_
#define _PHANTOM_SLAYER_MAIN_HEADER_

/*
 * C++ HEADERS
 */
#include "picosystem.hpp"
#include "hardware/adc.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "gfx.h"
#include "map.h"
#include "phantom.h"
#include "tinymt32.h"
#include "utils.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 *      CONSTANTS
 */
// Player movement directions
#define DIRECTION_NORTH                                 0
#define DIRECTION_EAST                                  1
#define DIRECTION_SOUTH                                 2
#define DIRECTION_WEST                                  3

#define MOVE_FORWARD                                    0
#define TURN_RIGHT                                      1
#define MOVE_BACKWARD                                   2
#define TURN_LEFT                                       3

#define NOT_IN_PLAY                                     0
#define IN_PLAY                                         1
#define PLAYER_IS_DEAD                                  2
#define START_COUNT                                     3
#define PLAY_INTRO                                      4
#define DO_TELEPORT_ONE                                 5
#define DO_TELEPORT_TWO                                 6
#define SHOW_TEMP_MAP                                   7

// Timer limits
#define PHANTOM_MOVE_TIME_US                            1000000
#define LASER_RECHARGE_US                               2000000
#define MAP_POST_KILL_SHOW_MS                           3000
#define LASER_FIRE_US                                   200000

// Map square types
#define MAP_TILE_WALL                                   0xEE
#define MAP_TILE_CLEAR                                  0xFF
#define MAP_TILE_TELEPORTER                             0xAA
#define MAX_VIEW_RANGE                                  5

#define MAX_PHANTOMS                                    3
#define ERROR_CONDITION                                 99
#define PHANTOM_NORTH                                   1
#define PHANTOM_EAST                                    2
#define PHANTOM_SOUTH                                   4
#define PHANTOM_WEST                                    8


/*
 * STRUCTURE DEFINITIONS
 */
typedef struct {
    uint8_t                 x;
    uint8_t                 y;
    uint8_t                 direction;
} Player;

typedef struct {
    bool                    show_reticule;
    bool                    can_fire;
    bool                    is_firing;

    std::vector<Phantom>    phantoms;
    uint32_t                phantom_speed;
    uint32_t                last_phantom_move;

    Player                  player;

    uint8_t                 state;
    uint8_t                 map;
    uint8_t                 audio_range;
    uint8_t                 tele_x;
    uint8_t                 tele_y;
    uint8_t                 start_x;
    uint8_t                 start_y;

    uint8_t                 view_mode;

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
 *      PROTOTYPES
 */
void        setup();
void        start_new_game();
void        init_game();
void        init_phantoms();
void        create_world();

void        update_world();
bool        move_phantoms();
void        manage_phantoms();

uint8_t     get_direction(uint8_t key_pressed);
uint8_t     get_facing_phantom(uint8_t range);
uint8_t     count_facing_phantoms(uint8_t range);


void        check_senses();
void        fire_laser();
void        do_teleport();

void        death();
void        show_scores();
uint8_t     fix_num_width(uint8_t value, uint8_t current);

void        beep();


/*
 *      GLOBALS
 */



#ifdef __cplusplus
}


#endif

#endif // _PHANTOM_SLAYER_MAIN_HEADER_
