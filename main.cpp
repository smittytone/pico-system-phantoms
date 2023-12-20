/*
 * phantom-slayer for Raspberry Pi Pico
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using namespace picosystem;


/*
 *  GLOBALS
 */
uint8_t     count_down = 5;
uint8_t     dead_phantom_index = NONE;
uint8_t     help_page_count = 0;
uint8_t     stab_count = 0;

int16_t     logo_y = -21;
int32_t     anim_x = 0;

uint32_t    tele_flash_time = 0;
uint32_t    tick_count = 0;

tinymt32_t  tinymt_store;

bool        chase_mode = false;
bool        map_mode = false;
bool        tele_state = false;

Rect        rects[7];

Game        game;

voice_t     blip = voice(10, 0, 40, 40);
voice_t     zap = voice(150, 0, 60, 350);
voice_t     stab = voice(10, 10, 300, 200);

/*
 *  EXTERNALLY-DEFINED GLOBALS
 */
extern      buffer_t* side_buffer;


/*
 * PICOSYSTEM CALLBACKS
 */

/**
 * @brief Picosystem callback function for
 *        initialisation routines.
 */
void init() {
    // Clear the display as soon as possible
    Gfx::cls(COLOURS::GREEN);

#ifdef DEBUG
    // Enable debugging
    stdio_init_all();
    sleep_ms(2000);
#endif

    // Show the version
    pen((color_t)COLOURS::BLACK);
    int32_t w;
    int32_t h;
    measure("1.1.3", w, h);
    cursor(238 - w, 238 - h);
    text("1.1.3");

    // Set up game device
    // NOTE This is all the stuff that is per session,
    //      not per game, or per level
    setup_device();
}


/**
 * @brief Picosystem callback function for
 *        game run loop.
 */
void update([[maybe_unused]] uint32_t tick_ms) {
    uint32_t now = time_us_32();
    uint8_t key = 0;

    if (game.is_dead) {
        death();
        return;
    }

    switch (game.state) {
        case GAME_STATE::ANIMATE_LOGO:
            if (now - tele_flash_time > LOGO_ANIMATION_US) {
                logo_y++;
                if (logo_y > 100) {
                    game.state = GAME_STATE::ANIMATE_CREDIT;
                    logo_y = 275;
                }
                tele_flash_time = now;
            }
            break;
        case GAME_STATE::ANIMATE_CREDIT:
            if (now - tele_flash_time > LOGO_ANIMATION_US) {
                logo_y--;
                if (logo_y < 130) {
                    game.state = GAME_STATE::LOGO_PAUSE;
                }
                tele_flash_time = now;
            }
            break;
        case GAME_STATE::LOGO_PAUSE:
            if (now - tele_flash_time > LOGO_PAUSE_TIME) {
                game.state = GAME_STATE::OFFER_HELP;
                tele_flash_time = 0;
            }
            break;
        case GAME_STATE::OFFER_HELP:
            key = Utils::inkey();
            if (key == (uint8_t)KEY::A) {
                //Help::init();
                Help::show_page(0);
                help_page_count = 0;
                game.state = GAME_STATE::SHOW_HELP;
                beep();
            } else if (key != 0) {
                start_new_game();
            }
            break;
        case GAME_STATE::SHOW_HELP:
            // Run through the help pages with each key press
            key = Utils::inkey();
            if (key > 0) {
                help_page_count += ((key == (uint8_t)KEY::B && help_page_count > 0) ? -1 : 1);
                if (help_page_count >= MAX_HELP_PAGES) {
                    start_new_game();
                    break;
                }

                if (help_page_count > MAX_HELP_PAGES - 1) help_page_count = 0;
            }
            break;
        case GAME_STATE::START_COUNT:
            // Count down five seconds before
            // actually starting the game
            tick_count++;
            if (tick_count % 50 == 0) {
                count_down--;
                beep();
            }

            if (count_down == 0) {
                tick_count = 0;
                game.state = GAME_STATE::IN_PLAY;
            }
            break;
        case GAME_STATE::PLAYER_IS_DEAD:
            // NOTE Call 'death()' before coming here
            // Just await any key press to start again
            tick_count++;
            if (tick_count > 200) {
                tick_count = 0;
                game.state = GAME_STATE::NEW_GAME_OFFER;
            }
            break;
        case GAME_STATE::NEW_GAME_OFFER:
            // Triggered after PLAYER_IS_DEAD has been visible
            if (Utils::inkey() > 0) start_new_game();
            break;
        case GAME_STATE::DO_TELEPORT_ONE:
            // Flip between TELE_ONE and TELE_TWO
            // every 1/4 second for two seconds
            tick_count++;
            if (tick_count == 5) {
                tick_count = 0;
                game.state = GAME_STATE::DO_TELEPORT_TWO;
                if (now - tele_flash_time > 1000000) {
                    // Half way through, switch co-ords
                    game.player.x = game.start_x;
                    game.player.y = game.start_y;
                }
            }
            break;
        case GAME_STATE::DO_TELEPORT_TWO:
            tick_count++;
            if (tick_count == 5) {
                tick_count = 0;
                game.state = now - tele_flash_time < 2000000 ? GAME_STATE::DO_TELEPORT_ONE : GAME_STATE::IN_PLAY;
            }
            break;
        case GAME_STATE::SHOW_TEMP_MAP:
            // Wait 3s while the post-kill
            // map is on screen
            tick_count++;
            if (tick_count == 150) {
                tick_count = 0;
                game.state = GAME_STATE::IN_PLAY;
            }
            break;
        case GAME_STATE::ANIMATE_RIGHT_TURN:
        case GAME_STATE::ANIMATE_LEFT_TURN:
            //anim_x += SLICE;
            if (anim_x > 240) {
                game.state = GAME_STATE::IN_PLAY;
                blend(ALPHA);
            }
            break;
        case GAME_STATE::ZAP_PHANTOM:
            tick_count++;
            if (tick_count == 25) {
                tick_count = 0;
                game.state = GAME_STATE::SHOW_TEMP_MAP;
                bool last_phantom_killed = (game.level_kills == game.phantom_count);
                phantom_killed(last_phantom_killed);

                // Take the dead phantom off the board
                // (so it gets re-rolled in `manage_phantoms()`)
                // NOTE `manage_phantoms()` calls `start_new_level()`
                //      if necessary
                Phantom& p = game.phantoms.at(dead_phantom_index);
                p.x = NOT_ON_BOARD;
                p.y = NOT_ON_BOARD;
                dead_phantom_index = NONE;
                manage_phantoms();
            }
            break;
        default:
            // The game is afoot! game.state = IN_PLAY
            // NOTE Return as quickly as possible

            // Was a key tapped?
            key = Utils::inkey();

            if ((key > (uint8_t)KEY::Y) && !game.show_reticule && !game.is_firing) {
                // A move key has been pressed
                MOVE dir = get_direction(key);
                uint8_t nx = game.player.x;
                uint8_t ny = game.player.y;

                if (dir == MOVE::FORWARD || dir == MOVE::BACKWARD) {
                    // Move player forward or backward if we can
                    if (game.player.direction == DIRECTION::NORTH) ny += (dir == MOVE::FORWARD ? -1 : 1);
                    if (game.player.direction == DIRECTION::SOUTH) ny += (dir == MOVE::FORWARD ? 1 : -1);
                    if (game.player.direction == DIRECTION::EAST) nx += (dir == MOVE::FORWARD ? 1 : -1);
                    if (game.player.direction == DIRECTION::WEST) nx += (dir == MOVE::FORWARD ? -1 : 1);

                    if (ny < 20 && nx < 20 && Map::get_square_contents(nx, ny) != MAP_TILE_WALL) {
                        // Has the player walked up to a Phantom?
                        if (Map::phantom_on_square(nx, ny) != NONE) {
                            // Yes -- so the player is dead!
                            //death();
                            game.is_dead = true;

                            #ifdef DEBUG
                            printf("\nPLAYER IS DEAD\n");
                            #endif
                            return;
                        }

                        // Set the new square for rendering later
                        game.player.x = nx;
                        game.player.y = ny;

                        #ifdef DEBUG
                        printf("MOVED PLAYER (KEY: %02x), DIRECTION: %u\n", key, (uint8_t)game.player.direction);
                        #endif
                    }
                } else if (dir == MOVE::RIGHT) {
                    // Turn player right
                    game.player.direction = do_turn_right(game.player.direction);
                    //game.player.direction += DIRECTION::EAST;
                    //if (game.player.direction > DIRECTION::WEST) game.player.direction = DIRECTION::NORTH;

                    // Animate the turn now
                    if (!chase_mode && !map_mode) {
                        #ifdef DEBUG
                        printf("TURNED PLAYER RIGHT, DIRECTION: %u\n", (uint8_t)game.player.direction);
                        #endif

                        anim_x = -SLICE;
                        game.state = GAME_STATE::ANIMATE_RIGHT_TURN;
                        Gfx::animate_turn();
                        return;
                    }
                } else if (dir == MOVE::LEFT) {
                    // Turn player left
                    game.player.direction = do_turn_left(game.player.direction);
                    //if (game.player.direction > DIRECTION::WEST) game.player.direction = DIRECTION::WEST;

                    // Animate the turn now
                    if (!chase_mode && !map_mode) {
                        #ifdef DEBUG
                        printf("TURNED PLAYER LEFT, DIRECTION: %u\n", (uint8_t)game.player.direction);
                        #endif

                        anim_x = -SLICE;
                        game.state = GAME_STATE::ANIMATE_LEFT_TURN;
                        Gfx::animate_turn();
                        return;
                    }
                }
            } else if ((key & (uint8_t)KEY::B) && !game.show_reticule && !game.is_firing) {
                // Player can only teleport if they have walked over the
                // teleport square and they are not firing the laser
                if (game.player.x == game.tele_x && game.player.y == game.tele_y) {
                    #ifdef DEBUG
                    printf("PLAYER TELEPORTING\n");
                    #endif

                    do_teleport();
                }
            } else if (key & (uint8_t)KEY::X) {
                #ifdef DEBUG
                // Map mode should be for debugging only
                map_mode = !map_mode;
                #endif

                // Lower radar range
                game.audio_range++;
                if (game.audio_range > 6) game.audio_range = 1;
                beep();

                #ifdef DEBUG
                printf("RADAR RANGE %u\n", game.audio_range);
                #endif
            } else if (key & (uint8_t)KEY::Y) {
                #ifdef DEBUG
                // Map mode should be for debugging only
                chase_mode = !chase_mode;
                #endif

                // Lower radar range
                game.audio_range--;
                if (game.audio_range < 1) game.audio_range = 6;
                beep();

                #ifdef DEBUG
                printf("RADAR RANGE %u\n", game.audio_range);
                #endif
            }

            // Check for firing
            // NOTE This uses separate code because it requires the button
            //      to be held down (fire on release)
            if (button(A)) {
                if (game.can_fire && !game.show_reticule) {
                    game.show_reticule = true;

                    #ifdef DEBUG
                    printf("READY TO FIRE\n");
                    #endif
                }
            } else {
                // Button released: check it was previously
                // pressed down, ie. 'game.show_reticule' is true
                if (game.show_reticule) {
                    // Fire the laser: clear the cross hair and zap
                    game.show_reticule = false;
                    reset_laser();
                    game.is_firing = true;

                    // Check if we've hit a Phantom
                    fire_laser();

                    #ifdef DEBUG
                        printf("FIRED\n");
                    #endif
                }
            }

            // Manage the world
            update_world();
    }
}


/**
 * @brief Picosystem callback function for
 *        graphics routines
 */
void draw([[maybe_unused]] uint32_t tick_ms) {

    switch(game.state) {
        case GAME_STATE::ANIMATE_LOGO:
            pen((color_t)COLOURS::GREEN);
            Gfx::animate_logo(logo_y);
            //play(piano, 2000 + ((logo_y + 20) * 10), 80, 30);
            break;
        case GAME_STATE::ANIMATE_CREDIT:
            pen((color_t)COLOURS::GREEN);
            Gfx::animate_credit(logo_y);
            //play(piano, 2000 + (300 - logo_y) * 100, 30);
            break;
        case GAME_STATE::LOGO_PAUSE:
            break;
        case GAME_STATE::OFFER_HELP:
            Help::show_offer();
            break;
        case GAME_STATE::SHOW_HELP:
            // Display a help page
            Help::show_page(help_page_count);
            break;
        case GAME_STATE::START_COUNT:
            // Update the on-screen countdown
            // NOTE The map is already there, so
            //      we only need to update the countdown
            //      itself
            // Clear the number
            pen((color_t)COLOURS::BLUE);
            frect(112, 214, 16, 20);

            // Show the new number
            pen((color_t)COLOURS::YELLOW);
            Gfx::draw_number(count_down, (count_down == 1 ? 118 : 114), 214, true);
            break;
        case GAME_STATE::SHOW_TEMP_MAP:
            // We've already drawn the post kill map, so just exit
        case GAME_STATE::PLAYER_IS_DEAD:
            // We've already drawn the end-of-game map, so just exit
            break;
        case GAME_STATE::NEW_GAME_OFFER:
            // Give instructions
            Gfx::draw_word(WORDS::ANY_KEY, 44, 215, true);
            break;
        case GAME_STATE::ANIMATE_RIGHT_TURN:
            anim_x += SLICE;
            if (anim_x > 240 - SLICE) break;

            // Blit screen left by one slice
            Gfx::alt_blit(SCREEN, SLICE, 40, 240 - SLICE, 160, 0, 40);

            // Blit side slice to last slice of screen
            Gfx::alt_blit(side_buffer, anim_x, 40, SLICE, 160, 240 - SLICE, 40);
            break;
        case GAME_STATE::ANIMATE_LEFT_TURN:
            anim_x += SLICE;
            if (anim_x > 240) break;

            for (int32_t x = 240 - (SLICE * 2) ; x >= 0 ; x -= SLICE) {
                Gfx::alt_blit(SCREEN, x, 40, SLICE, 160, x + SLICE, 40);
            }

            // Blit side slice to last slice of screen
            Gfx::alt_blit(side_buffer, 240 - anim_x, 40, SLICE, 160, 0, 40);
            break;
        default:
            // Render the screen
            if (chase_mode) {
                // Show the first Phantom's view
                const Phantom& p = game.phantoms.at(0);
                Gfx::draw_screen(p.x, p.y, (uint8_t)p.direction);
            } else if (map_mode) {
                // Draw an overhead view
                Map::draw(BASE_MAP_Y_DELTA, true);
            } else {
                // Show the player's view
                Gfx::draw_screen(game.player.x, game.player.y, (uint8_t)game.player.direction);
            }

            // Don't show gunnery if a Phantom has been hit
            if (game.state == GAME_STATE::ZAP_PHANTOM) return;

            // Is the laser being fired?
            if (game.is_firing) Gfx::draw_zap(game.zap_frame);

            // Has the player primed the laser?
            if (game.show_reticule) Gfx::draw_reticule();
    }
}


/*
 * INITIALISATION FUNCTIONS
 */

/**
 * @brief Initial hardware setup and fundamental config
 *        (power-cycle level stuff)
 */
void setup_device(void) {

    // Set the LCD backlight
    // TODO Make this controllable during the game
    backlight(100);

    // Make the graphic frame rects
    // NOTE These are pixel values:
    //      left, top, width, height, Phantom lateral offset
    uint8_t coords[] = { 0,    0, 240, 160, 60,     // Outer LED frame
                         20,   9, 200, 142, 50,
                         44,  20, 152, 120, 38,
                         66,  30, 108, 100, 26,
                         88,  40,  64,  80, 20,
                         102, 46,  36,  68,  8,
                         114, 51,  12,  58,  2};    // 'End wall' for distant views

    // Read the array values into Rect structures
    uint8_t c = 0;
    for (uint8_t i = 0 ; i < sizeof(coords) ; i += 5) {
        Rect a_rect;
        a_rect.x = coords[i];
        a_rect.y = coords[i + 1];
        a_rect.width = coords[i + 2];
        a_rect.height = coords[i + 3];
        a_rect.spot = coords[i + 4];
        rects[c++] = a_rect;
    }

    // Start the game loop at the intro animation
    game.state = GAME_STATE::ANIMATE_LOGO;

    #ifdef DEBUG
    printf("DONE SETUP_DEVICE\n");
    #endif
}


/**
 * @brief Start a new game by re-initialising the game state,
 *        and setting up a new maze. Called at the start of the
 *        first game and subsequently when the player dies.
 */
void start_new_game() {

    // Reset the settings
    init_game();
    init_phantoms();
    start_new_level();

    // Clear the screen (blue), present the current map
    // and give the player a five-second countdown before
    // entering the maze
    Gfx::cls(COLOURS::BLUE);
    Map::draw(BASE_MAP_Y_DELTA, false);

    Gfx::draw_number(game.level, 156, 12, true);
    Gfx::draw_word(WORDS::LEVEL, 72, 12, true);

    // Set the game mode
    game.state = GAME_STATE::START_COUNT;
    count_down = 5;

    #ifdef DEBUG
    printf("DONE START_NEW_GAME()\n");
    #endif
}


/**
 * @brief Reset the main game control structure. Called only at the start of
 *        a game, not the start of a level.
 *
 *        NOTE Phantom data is separated out into `init_phantoms()`.
 */
void init_game(void) {

    // If either of these demo/test modes are both set,
    // chase mode takes priority
    chase_mode = false;
    map_mode = false;

    // FROM 1.0.2
    // Store the current map number so it's not
    // used in the next game
    game.map = NONE;
    game.phantom_count = 1;

    game.player.x = 0;
    game.player.y = 0;
    game.player.direction = DIRECTION::NORTH;

    game.tele_x = 0;
    game.tele_y = 0;
    game.start_x = 0;
    game.start_y = 0;

    game.level = 1;
    game.score = 0;
    game.kills = 0;
    game.high_score = 0;

    game.crosshair_delta = 0;
    game.audio_range = 4;

    game.is_dead = false;

    #ifdef DEBUG
    printf("DONE INIT_GAME()\n");
    #endif

    // FROM 1.1.3 -- Move this here
    // Randomise using TinyMT
    // https://github.com/MersenneTwister-Lab/TinyMT
    tinymt32_init(&tinymt_store, Utils::get_seed());
}


/**
 * @brief Reset the main game control structure values
 *        for the start of a level.
 */
void init_level(void) {

    game.show_reticule = false;
    game.can_fire = true;
    game.is_firing = false;

    game.level_kills = 0;
    game.level_hits = 0;

    game.zap_charge_time = 0;
    game.zap_fire_time = 0;
    game.zap_frame = 0;

    #ifdef DEBUG
    printf("DONE INIT_LEVEL()\n");
    #endif
}


/**
 * @brief Initialise the current game's Phantom data.
 */
void init_phantoms(void) {

    // Reset the array stored phantoms structures
    game.phantoms.clear();
    for (uint8_t i = 0 ; i < MAX_PHANTOMS ; i++) {
        Phantom p;
        game.phantoms.push_back(p);
    }

    game.phantom_speed = PHANTOM_MOVE_TIME_US << 1;
    game.last_phantom_move = 0;

    #ifdef DEBUG
    printf("DONE INIT_PHANTOMS()\n");
    #endif
}


/**
 * @brief Generate and populate a new level. This happens
 *        at the start of a new game and at the start of
 *        each level. A level jump is triggered when all the
 *        current phantoms have been dispatched.
 */
void start_new_level(void) {
    // Initialise the current map
    game.map = Map::init(game.map);
    init_level();

    // Place the player near the centre
    uint8_t x = 9;
    uint8_t y = 9;

    while (true) {
        x = 9 + (Utils::irandom(0, 3) - 1);
        y = 9 + (Utils::irandom(0, 3) - 1);
        if (Map::get_square_contents(x, y) == MAP_TILE_CLEAR) break;
    }

    game.player.x = x;
    game.player.y = y;
    game.player.direction = static_cast<DIRECTION>(Utils::irandom(0, 4));
    //if (game.player.direction > 3) game.player.direction = 1
    game.start_x = x;
    game.start_y = y;

    #ifdef DEBUG
    printf("PLAYER LOCATION: %u,%u. DIRECTION: %u\n", x, y, (uint8_t)game.player.direction);
    #endif

    // Set the teleport
    set_teleport_square();

    // Add the first phantom to the map, everywhere but empty
    // or where the player has been placed.
    // NOTE 'game.phantom_count' set in 'init_game()' and 'manage_phantoms()'
    for (uint8_t i = 0 ; i < game.phantom_count ; i++) {
        Phantom& p = game.phantoms.at(i);
        p.init();
        p.place(i);
    }

    /* TEST DATA
    game.player.x = 0;
    game.player.y = 0;
    game.player.direction = DIRECTION::EAST;
     */

    #ifdef DEBUG
    printf("DONE START_NEW_LEVEL()\n");
    #endif
}


/**
 * @brief Randomly roll a teleport square.
 */
void set_teleport_square(void) {

    while (true) {
        // Pick a random co-ordinate
        uint8_t x = Utils::irandom(0, 20);
        uint8_t y = Utils::irandom(0, 20);

        if (Map::get_square_contents(x, y) == MAP_TILE_CLEAR && x != game.player.x && y != game.player.y) {
            game.tele_x = x;
            game.tele_y = y;
            break;
        }
    }
}


/**
 * @brief Update the world at the end of the move cycle.
 *        Called from the main `update()` callback.
 */
void update_world(void ) {

    // Move the Phantom(s) periodically -- this is how
    // we increase their speed as the game progresses
    uint32_t now = time_us_32();
    if (now - game.last_phantom_move > game.phantom_speed) {
        game.last_phantom_move = now;
        if (!move_phantoms()) {
            check_senses();
        } else {
            // Player was killed
            //death();
            game.is_dead = true;

            #ifdef DEBUG
            printf("PLAYER IS DEAD\n");
            #endif

            return;
        }
    }

    // Check for a laser recharge
    if (now - game.zap_charge_time > LASER_RECHARGE_US) {
        game.zap_charge_time = 0;
        game.can_fire = true;
    }

    // Animate the laser zap
    if (game.is_firing && now - game.zap_fire_time > LASER_FIRE_US) {
        if (game.zap_frame == 6) {
            reset_laser();
        } else {
            game.zap_frame++;
            game.zap_fire_time = now;
        }
    }
}


/**
 * @brief Check whether we need to increase the number of phantoms
 *        on the board or increase their speed -- all caused by a
 *        level-up. We up the level if all the level's phantoms have
 *        been zapped.
 */
void manage_phantoms(void) {

    bool level_up = false;

    // If we're on levels 1 and 2, we only have that number of
    // Phantoms. From 3 and up, there are aways three in the maze
    if (game.level < MAX_PHANTOMS) {
        if (game.level_kills == game.level) {
            level_up = true;
            game.level++;
            game.phantom_count = game.level;
        }
    } else {
        if (game.level_kills == MAX_PHANTOMS) {
            level_up = true;
            game.level++;
            game.phantom_count = MAX_PHANTOMS;
        }
    }

    // Did we level-up? Is so, update the phantom movement speed
    if (level_up) {
        uint8_t index = (game.level - 1) * 4;
        game.phantom_speed = ((PHANTOM_MOVE_TIME_US << level_data[index + 2]) >> level_data[index + 3]);

        // Just in case...
        if (game.phantom_count > MAX_PHANTOMS) game.phantom_count = MAX_PHANTOMS;

        // Take all existing Phantoms off the board
        for (uint8_t i = 0 ; i < game.phantom_count ; i++) {
            Phantom& p = game.phantoms.at(i);
            p.x = NONE;
            p.y = NONE;
        }

        // Create a new level
        start_new_level();
    }
}


/**
 * @brief Get player movement action from the joypad
 *        Favour movement over rotation.
 *
 * @param keys_pressed: Bitfield indicating which
 *                      keys have been pressed.
 *
 * @returns: The direction in which the player is facing.
 */
MOVE get_direction(uint8_t keys_pressed) {

    if (keys_pressed & (uint8_t)KEY::UP) return MOVE::FORWARD;
    if (keys_pressed & (uint8_t)KEY::DOWN) return MOVE::BACKWARD;
    if (keys_pressed & (uint8_t)KEY::LEFT) return MOVE::LEFT;
    if (keys_pressed & (uint8_t)KEY::RIGHT) return MOVE::RIGHT;

    // Just in case
    return MOVE::ERROR_CONDITION;
}


/**
 * @brief Return the index of the closest facing Phantom to the
 *        player from the the 'game.phantoms' vector --
 *        or `NONE`.
 *
 * @param range: The number of squares to iterate over.
 *
 * @returns: The index of the Phantom in the vector.
 */
uint8_t get_facing_phantom(uint8_t range) {

    uint8_t p_index = NONE;
    switch(game.player.direction) {
        case DIRECTION::NORTH:
            if (game.player.y == 0) return NONE;
            if (game.player.y - range < 0) range = game.player.y;
            for (int8_t i = game.player.y ; i > game.player.y - range ; --i) {
                p_index = Map::phantom_on_square(game.player.x, i);
                if (p_index != NONE) return p_index;
            }
            break;
        case DIRECTION::EAST:
            if (game.player.x == MAP_MAX) return NONE;
            if (game.player.x + range > MAP_MAX) range = MAP_MAX - game.player.x;
            for (int8_t i = game.player.x ; i < game.player.x + range ; ++i) {
                p_index = Map::phantom_on_square(i, game.player.y);
                if (p_index != NONE) return p_index;
            }
            break;
        case DIRECTION::SOUTH:
            if (game.player.y == MAP_MAX) return NONE;
            if (game.player.y + range > MAP_MAX) range = MAP_MAX - game.player.y;
            for (int8_t i = game.player.y ; i < game.player.y + range ; ++i) {
                p_index = Map::phantom_on_square(game.player.x, i);
                if (p_index != NONE) return p_index;
            }
            break;
        default:
            if (game.player.x == 0) return NONE;
            if (game.player.x - range < 0) range = game.player.x;
            for (int8_t i = game.player.x ; i > game.player.x - range ; --i) {
                p_index = Map::phantom_on_square(i, game.player.y);
                if (p_index != NONE) return p_index;
            }
    }

    return p_index;
}


/**
 * @brief Return the number of Phantoms in front of the player.
 *
 * @param range: The number of squares to iterate over.
 *
 * @returns: The number of Phantoms in front of the Player.
 */
uint8_t count_facing_phantoms(uint8_t range) {

    uint8_t phantom_count = 0;
    switch(game.player.direction) {
        case DIRECTION::NORTH:
            if (game.player.y == 0) return phantom_count;
            if (game.player.y - range < 0) range = game.player.y;
            for (int8_t i = game.player.y ; i >= game.player.y - range ; --i) {
                phantom_count += (Map::phantom_on_square(game.player.x, i) != NONE ? 1 : 0);
            }
            break;
        case DIRECTION::EAST:
            if (game.player.x == MAP_MAX) return phantom_count;
            if (game.player.x + range > MAP_MAX) range = MAP_MAX - game.player.x;
            for (int8_t i = game.player.x ; i <= game.player.x + range ; ++i) {
                phantom_count += (Map::phantom_on_square((uint8_t)i, game.player.y) != NONE ? 1 : 0);
            }
            break;
        case DIRECTION::SOUTH:
            if (game.player.y == MAP_MAX) return phantom_count;
            if (game.player.y + range > MAP_MAX) range = MAP_MAX - game.player.y;
            for (int8_t i = game.player.y ; i <= game.player.y + range ; ++i) {
                phantom_count += (Map::phantom_on_square(game.player.x, i) != NONE ? 1 : 0);
            }
            break;
        default:
            if (game.player.x == 0) return phantom_count;
            if (game.player.x - range < 0) range = game.player.x;
            for (int8_t i = game.player.x ; i >= game.player.x - range ; --i) {
                phantom_count += (Map::phantom_on_square(i, game.player.y) != NONE ? 1 : 0);
            }
    }

    return phantom_count;
}


/**
 * @brief Tell all of the current Phantoms to move.
 *
 * @returns: `true` if a Phantom caught the Player,
 *           otherwise `false`.
*/
bool move_phantoms(void) {

    bool player_is_caught = false;
    for (uint8_t i = 0 ; i < MAX_PHANTOMS ; i++) {
        Phantom& p = game.phantoms.at(i);
        if (p.move()) player_is_caught = true;
    }

    return player_is_caught;
}


/**
 * @brief Scan around the player for nearby Phantoms.
 */
void check_senses(void) {

    int dx = game.player.x - game.audio_range;
    int dy = game.player.y - game.audio_range;

    for (int x = dx ; x < (dx + (game.audio_range << 1)) ; x++) {
        if (x < 0) continue;
        if (x > MAP_MAX) break;
        for (int y = dy ; y < (dy + (game.audio_range << 1)) ; y++) {
            if (y < 0) continue;
            if (y > MAP_MAX) break;
            uint8_t nabbed = Map::phantom_on_square(x, y);
            if (nabbed != NONE) {
                // There's a Phantom in range, so sound a tone
                led(100, 0, 0);
                beep();
                sleep_ms(200);
                led(0, 0, 0);

                // Only play one beep, no matter
                // how many nearby phantoms there are
                return;
            }
        }
    }
}


/**
 * @brief Simple beep routine.
 */
void beep(void) {

    play(blip, 200, 50);
}


/*
 * ACTIONS
 */

/**
 * @brief Jump back to the teleport square if the player has walked over it.
 */
void do_teleport(void) {

    // Move the player to the stored square
    game.state = GAME_STATE::DO_TELEPORT_ONE;
    tele_flash_time = time_us_32();

    // Reset the laser if it's firing
    reset_laser();

    // Re-locate the teleport square
    set_teleport_square();
}


/**
 * @brief Hit the front-most facing Phantom, if there is one.
 */
void fire_laser(void) {

    // Did we hit a Phantom?
    play(zap, 640, 200);
    uint8_t index = get_facing_phantom(MAX_VIEW_RANGE);
    if (index != NONE) {
        // A hit! A palpable hit!
        // Deduct 1HP from the Phantom
        Phantom& p = game.phantoms.at(index);
        p.hp--;

        // FROM 1.0.2
        // Use original scoring: 2 points for a hit, 10 for a kill
        game.score += 2;
        game.level_hits++;
        game.is_firing = false;

        // Did that kill it?
        if (p.hp < 1) {
            // Yes! One dead Phantom...
            p.hp = 0;
            //p.x = NOT_ON_BOARD;
            //p.y = NOT_ON_BOARD;
            game.score += 10;
            game.level_kills++;
            game.kills++;

            // Briefly sound some tones
            // tone(1200, 100, 200);
            // tone(600, 100, 200);

            // Quickly show the map
            game.state = GAME_STATE::ZAP_PHANTOM;
            dead_phantom_index = index;

            // Reset the laser
            reset_laser();
        }
    }
}


/**
 * @brief Reset the laser after firing.
 */
void reset_laser(void) {

    game.is_firing = false;
    game.can_fire = false;
    game.zap_charge_time = time_us_32();
    game.zap_frame = 0;
}


/*
 * GAME OUTCOMES
 */

/**
 * @brief The player has died -- show the map and the score.
 */
void death(void) {

    game.state = GAME_STATE::PLAYER_IS_DEAD;
    game.is_dead = false;
    //for (unsigned int i = 400 ; i > 100 ; i -= 2) tone(i, 30, 0);
    //sleep_ms(50);
    //tone(2200, 500, 600);

    // Clear the display (blue)
    Gfx::cls(COLOURS::BLUE);

    // Show the map
    show_scores(true);

    // Clear a space and say what happened
    pen((color_t)COLOURS::BLUE);
    frect(36, 106, 168, 28);
    Gfx::draw_word(WORDS::PLAYER_DEAD, 38, 110, true);
}


/**
 * @brief Just show the map briefly after killing a Phantom.
 *
 * @param is_last: Is this the last Phantom of a group?
 */
void phantom_killed(bool is_last) {

    Gfx::cls(COLOURS::BLUE);
    show_scores(is_last);
}


/**
 * @brief Code used in a couple of 'show map' locations
 *        Show the current score alongside the map.
 *
 * @param do_show_teleport: Should the teleport square be included?
 */
void show_scores(bool do_show_teleport) {

    uint8_t cx = 10;
    if (game.high_score < game.score) game.high_score = game.score;

    // Show the score
    Gfx::draw_word(WORDS::SCORE, 10, 5, false);
    uint32_t score = Utils::bcd(game.score);
    Gfx::draw_number((score & 0xF000) >> 12, cx, 18, true);
    cx += (((score & 0xF000) >> 12) == 1 ? 6 : 14);
    Gfx::draw_number((score & 0x0F00) >> 8, cx, 18, true);
    cx += (((score & 0x0F00) >> 8) == 1 ? 6 : 14);
    Gfx::draw_number((score & 0x00F0) >> 4, cx, 18, true);
    cx += (((score & 0x00F0) >> 4) == 1 ? 6 : 14);
    Gfx::draw_number(score & 0x000F, cx, 18, true);

    // Show the high score
    Gfx::draw_word(WORDS::HIGH, 162, 5, false);
    Gfx::draw_word(WORDS::SCORE, 192, 5, false);
    score = Utils::bcd(game.high_score);
    cx = ((score & 0x000F) == 1) ? 226 : 218;
    Gfx::draw_number(score & 0x000F, cx, 18, true);
    cx = fix_num_width((score & 0x00F0) >> 4, cx);
    Gfx::draw_number((score & 0x00F0) >> 4,  cx, 18, true);
    cx = fix_num_width((score & 0x0F00) >> 8, cx);
    Gfx::draw_number((score & 0x0F00) >> 8,  cx, 18, true);
    cx = fix_num_width((score & 0xF000) >> 12, cx);
    Gfx::draw_number((score & 0xF000) >> 12, cx, 18, true);

    if (game.state != GAME_STATE::PLAYER_IS_DEAD) {
        // This is for the intermediate map only
        // Show kills
        Gfx::draw_word(WORDS::KILLS, 198, 228, false);
        score = Utils::bcd(game.level_kills);
        cx = (score == 1) ? 226 : 220;
        Gfx::draw_number(score, cx, 204, true);

        // Show hits
        Gfx::draw_word(WORDS::HITS, 10, 228, false);
        score = Utils::bcd(game.level_hits);
        Gfx::draw_number(score, 10, 204, true);
    }

    // Add in the map
    Map::draw(BASE_MAP_Y_DELTA, true, do_show_teleport);
}


/**
 * @brief Return the delta when presenting a number.
 *        Lower for 1, bigger for 0, 2-9
 */
uint8_t fix_num_width(uint8_t value, uint8_t current) {

    return (current - (value == 1 ? 6 : 14));
}


inline DIRECTION do_turn_right(DIRECTION current) {

    auto a = (uint8_t)current;
    a++;
    if (a > (uint8_t)DIRECTION::WEST) return DIRECTION::NORTH;
    return (DIRECTION)a;
}


inline DIRECTION do_turn_left(DIRECTION current) {

    auto a = (uint8_t)current;
    a--;
    if (a > (uint8_t)DIRECTION::WEST) return DIRECTION::WEST;
    return (DIRECTION)a;
}


