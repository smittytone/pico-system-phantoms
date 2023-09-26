/*
 * phantom-slayer for Raspberry Pi Pico
 *
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
#include "hardware/structs/rosc.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#include "gfx.h"
#include "help.h"
#include "map.h"
#include "phantom.h"
#include "tinymt32.h"
#include "utils.h"


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

// Game states
enum {
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
    PLAYER_DEAD_NEXT_GAME,
    ANIMATE_RIGHT_TURN,
    ANIMATE_LEFT_TURN
};

// Timer limits
#define PHANTOM_MOVE_TIME_US                            1000000
#define LASER_RECHARGE_US                               2000000
#define MAP_POST_KILL_SHOW_MS                           3000
#define LASER_FIRE_US                                   200000
#define LOGO_ANIMATION_US                               9000
#define LOGO_PAUSE_TIME                                 5000000
// FROM 1.1.3
#define DEATH_MAP_INTERVAL                              4000

// Map square types
#define MAP_TILE_WALL                                   0xEE
#define MAP_TILE_CLEAR                                  0xFF
#define MAP_TILE_TELEPORTER                             0xAA
#define MAX_VIEW_RANGE                                  5
#define BASE_MAP_DELTA                                  4

#define MAX_PHANTOMS                                    3
#define ERROR_CONDITION                                 99
#define PHANTOM_NORTH                                   1
#define PHANTOM_EAST                                    2
#define PHANTOM_SOUTH                                   4
#define PHANTOM_WEST                                    8

// Colours
#define GREEN                                           0xF0F0
#define BLUE                                            0x0FF0
#define RED                                             0x00FF
#define WHITE                                           0xFFFF
#define YELLOW                                          0xF0FF
#define ORANGE                                          0xA0FF
#define CLEAR                                           0x0000
#define BLACK                                           0x00F0

// Turn animation screen slice size
#define SLICE                                           16


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
    uint8_t                 phantom_count;
    uint32_t                phantom_speed;
    uint32_t                last_phantom_move;
    int8_t                  crosshair_delta;

    Player                  player;

    uint8_t                 state;
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


#ifdef __cplusplus
extern "C" {
#endif


/*
 *      PROTOTYPES
 */

uint8_t     count_facing_phantoms(uint8_t range);


#ifdef __cplusplus
}


#endif

#endif // _PHANTOM_SLAYER_MAIN_HEADER_
