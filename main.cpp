/*
 * phantom-slayer for Raspberry Pi Pico
 *
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#include "main.h"

using namespace picosystem;


/*
 * STATIC PROTOTYPES
 */
static void setup_device(void);
static void start_new_game(void);
static void init_game(void);
static void start_new_level(void);
static void init_level(void);
static void init_phantoms(void);
static void set_teleport_square(void);
static void update_world(void);
static void check_senses(void);
static void manage_phantoms(void);
static bool move_phantoms(void);
static void do_teleport(void);
static void fire_laser(void);
static void reset_laser(void);
static void death(void);
static void phantom_killed(bool is_last = false);
static void show_scores(bool show_tele = false);
static uint8_t get_direction(uint8_t key_pressed);
static uint8_t get_facing_phantom(uint8_t range);



/*
 *  GLOBALS
 */
uint8_t     count_down = 5;
uint8_t     dead_phantom = ERROR_CONDITION;
int8_t      help_page_count = 0;
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
voice_t     roar = voice(5, 5, 100, 500, 0, 100, 100);

/*
 *  EXTERNALLY-DEFINED GLOBALS
 */
extern      buffer_t* side_buffer;


/*
 *  PICOSYSTEM CALLBACKS
 */
void init(void) {

    // Clear the display as soon as possible
    Gfx::cls(GREEN);

#ifdef DEBUG
    // Enable debugging
    stdio_init_all();
    sleep_ms(2000);
#endif

    // Show the version
    pen(BLACK);
    int32_t w, h;
    measure("1.1.3", w, h);
    cursor(238 - w, 238 - h);
    text("1.1.3");

    // Set up game device
    // NOTE This is all the stuff that is per session,
    //      not per game, or per level
    setup_device();

    // FROM 1.1.3
    // Clear High Score here, on boot
    game.high_score = 0;
}


void update(uint32_t tick_ms) {

    uint32_t now = time_us_32();
    uint8_t key = 0;
    switch (game.state) {
        case ANIMATE_LOGO:
            if (now - tele_flash_time > LOGO_ANIMATION_US) {
                logo_y++;
                if (logo_y > 100) {
                    game.state = ANIMATE_CREDIT;
                    logo_y = 275;
                }
                tele_flash_time = now;
            }
            break;
        case ANIMATE_CREDIT:
            if (now - tele_flash_time > LOGO_ANIMATION_US) {
                logo_y--;
                if (logo_y < 130) {
                    game.state = LOGO_PAUSE;
                }
                tele_flash_time = now;
            }
            break;
        case LOGO_PAUSE:
            if (now - tele_flash_time > LOGO_PAUSE_TIME) {
                game.state = OFFER_HELP;
                tele_flash_time = 0;
            }
            break;
        case OFFER_HELP:
            key = Utils::inkey();
            if (key == 0x01) {
                //Help::init();
                Help::show_page(0);
                help_page_count = 0;
                game.state = SHOW_HELP;
                Utils::beep();
            } else if (key != 0) {
                start_new_game();
            }
            break;
        case SHOW_HELP: {
            // Run through the help pages with each key press
            uint32_t k = Utils::inkey();
            if (k > 0) {
                help_page_count += (k != 2 ? 1 : -1);
                if (help_page_count < 0) help_page_count = 0;
                Utils::beep();
            }

            if (help_page_count >= MAX_HELP_PAGES) start_new_game();
            break;
        }
        case START_COUNT:
            // Count down five seconds before
            // actually starting the game
            tick_count++;
            if (tick_count % 50 == 0) {
                count_down--;
                Utils::beep();
            }

            if (count_down == 0) {
                tick_count = 0;
                game.state = IN_PLAY;
            }
            break;
        case PLAYER_IS_DEAD:
            // NOTE Call 'death()' before coming here
            // Clear a space and say what happened
            tick_count++;
            if (tick_count == 100) {
                pen(BLUE);
                frect(36, 106, 168, 28);
                Gfx::draw_word(PHRASE_PLAYER_DEAD, 38, 110, true);
                game.state = PLAYER_DEAD_NEXT_GAME;
                tick_count = 0;
            };
            break;
        case PLAYER_DEAD_NEXT_GAME:
            // FROM 1.1.3
            // Just await any key press to start again
            tick_count++;
            if (tick_count == 100) {
                pen(BLUE);
                frect(36, 106, 168, 28);
                Gfx::draw_word(PHRASE_ANY_KEY, 44, 110, true);
            };;
            if (Utils::inkey() > 0) start_new_game();
            break;
        case DO_TELEPORT_ONE:
            // Flip between TELE_ONE and TELE_TWO
            // every 1/4 second for two seconds
            tick_count++;
            if (tick_count == 5) {
                tick_count = 0;
                game.state = DO_TELEPORT_TWO;
                if (now - tele_flash_time > 1000000) {
                    // Half way through, switch co-ords
                    game.player.x = game.start_x;
                    game.player.y = game.start_y;
                }
            }
            break;
        case DO_TELEPORT_TWO:
            tick_count++;
            if (tick_count == 5) {
                tick_count = 0;
                game.state = now - tele_flash_time < 2000000 ? DO_TELEPORT_ONE : IN_PLAY;
            }
            break;
        case SHOW_TEMP_MAP:
            // Wait 3s while the post-kill
            // map is on screen
            tick_count++;
            if (tick_count == 150) {
                tick_count = 0;
                game.state = IN_PLAY;
            }
            break;
        case ANIMATE_RIGHT_TURN:
        case ANIMATE_LEFT_TURN:
            if (anim_x > 240) {
                game.state = IN_PLAY;
                blend(ALPHA);
            }
            break;
        case ZAP_PHANTOM:
            tick_count++;
            if (tick_count == 25) {
                tick_count = 0;
                game.state = SHOW_TEMP_MAP;
                bool last_phantom_killed = (game.level_kills == game.phantom_count);
                phantom_killed(last_phantom_killed);

                // Take the dead phantom off the board
                // (so it gets re-rolled in `manage_phantoms()`)
                // NOTE `manage_phantoms()` calls `start_new_level()`
                //      if necessary
                Phantom &p = game.phantoms.at(dead_phantom);
                p.x = ERROR_CONDITION;
                p.y = ERROR_CONDITION;
                dead_phantom = ERROR_CONDITION;
                manage_phantoms();
            }
        default:
            // The game is afoot! game.state = IN_PLAY
            // NOTE Return as quickly as possible

            // Was a key tapped?
            key = Utils::inkey();

            if ((key > 0x0F) && !game.show_reticule) {
                // A move key has been pressed
                uint8_t dir = get_direction(key);
                uint8_t nx = game.player.x;
                uint8_t ny = game.player.y;

                if (dir == MOVE_FORWARD || dir == MOVE_BACKWARD) {
                    // Move player forward or backward if we can
                    if (game.player.direction == DIRECTION_NORTH) ny += (dir == MOVE_FORWARD ? -1 : 1);
                    if (game.player.direction == DIRECTION_SOUTH) ny += (dir == MOVE_FORWARD ? 1 : -1);
                    if (game.player.direction == DIRECTION_EAST) nx += (dir == MOVE_FORWARD ? 1 : -1);
                    if (game.player.direction == DIRECTION_WEST) nx += (dir == MOVE_FORWARD ? -1 : 1);

                    if (ny < 20 && nx < 20 && Map::get_square_contents(nx, ny) != MAP_TILE_WALL) {
                        // Has the player walked up to a Phantom?
                        if (Map::phantom_on_square(nx, ny) != ERROR_CONDITION) {
                            // Yes -- so the player is dead!
                            death();

                            #ifdef DEBUG
                            printf("\nPLAYER IS DEAD\n");
                            #endif
                            return;
                        }

                        // Set the new square for rendering later
                        game.player.x = nx;
                        game.player.y = ny;

                        #ifdef DEBUG
                        printf("MOVED PLAYER (KEY: %02x), DIRECTION: %i\n", key, game.player.direction);
                        #endif
                    }
                } else if (dir == TURN_RIGHT) {
                    // Turn player right
                    game.player.direction++;
                    if (game.player.direction > DIRECTION_WEST) game.player.direction = DIRECTION_NORTH;

                    // Animate the turn now
                    if (!chase_mode && !map_mode) {
                        #ifdef DEBUG
                        printf("TURNED PLAYER RIGHT, DIRECTION: %i\n", game.player.direction);
                        #endif

                        anim_x = -SLICE;
                        game.state = ANIMATE_RIGHT_TURN;
                        Gfx::animate_turn();
                        return;
                    }
                } else if (dir == TURN_LEFT) {
                    // Turn player left
                    game.player.direction--;
                    if (game.player.direction > DIRECTION_WEST) game.player.direction = DIRECTION_WEST;

                    // Animate the turn now
                    if (!chase_mode && !map_mode) {
                        #ifdef DEBUG
                        printf("TURNED PLAYER LEFT, DIRECTION: %i\n", game.player.direction);
                        #endif

                        anim_x = -SLICE;
                        game.state = ANIMATE_LEFT_TURN;
                        Gfx::animate_turn();
                        return;
                    }
                }
            } else if ((key & 0x02) && !game.show_reticule) {
                // Player can only teleport if they have walked over the
                // teleport square and they are not firing the laser
                if (game.player.x == game.tele_x && game.player.y == game.tele_y) {
                    #ifdef DEBUG
                    printf("PLAYER TELEPORTING\n");
                    #endif

                    do_teleport();
                }
            } else if (key & 0x04) {
                #ifdef DEBUG
                // Map mode should be for debugging only
                map_mode = !map_mode;
                #endif

                // Lower radar range
                game.audio_range++;
                if (game.audio_range > 6) game.audio_range = 1;
                Utils::beep();

                #ifdef DEBUG
                printf("RADAR RANGE %i\n", game.audio_range);
                #endif
            } else if (key & 0x08) {
                // Lower radar range
                game.audio_range--;
                if (game.audio_range < 1) game.audio_range = 6;
                Utils::beep();

                #ifdef DEBUG
                printf("RADAR RANGE %i\n", game.audio_range);
                #endif
            }

            // Check for firing
            // NOTE This uses separate code because it requires the button
            //      to be held down (fire on release)
            if (button(A)) {
                if (game.can_fire) {
                    // Button A pressed
                    if (!game.show_reticule) {
                        game.show_reticule = true;

                        #ifdef DEBUG
                        printf("READY TO FIRE\n");
                        #endif
                    }
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


void draw(uint32_t tick_ms) {

    uint8_t nx;
    buffer_t* scrn = SCREEN;
    switch(game.state) {
        case ANIMATE_LOGO:
            pen(GREEN);
            Gfx::animate_logo(logo_y);
            play(roar, logo_y + 125);
            break;
        case ANIMATE_CREDIT:
            pen(GREEN);
            Gfx::animate_credit(logo_y);
            play(roar, 525 - logo_y);
            break;
        case LOGO_PAUSE:
            break;
        case OFFER_HELP:
            Help::show_offer();
            break;
        case SHOW_HELP:
            // Display a help page
            Help::show_page(help_page_count);
            break;
        case START_COUNT:
            // Update the on-screen countdown
            // NOTE The map is already there, so
            //      we only need to update the countdown
            //      itself
            // Clear the number
            pen(BLUE);
            frect(112, 214, 16, 20);

            // Show the new number
            pen(YELLOW);
            Gfx::draw_number(count_down, (count_down == 1 ? 118 : 114), 214, true);
            break;
        case SHOW_TEMP_MAP:
            // We've already drawn the post kill map, so just exit
        case PLAYER_IS_DEAD:
        case PLAYER_DEAD_NEXT_GAME:
            // We've already drawn the end-of-game map, so just exit
            break;
        case ANIMATE_RIGHT_TURN:
            anim_x += SLICE;
            if (anim_x > 240 - SLICE) break;

            // Blit screen left by one slice
            Gfx::alt_blit(SCREEN, SLICE, 40, 240 - SLICE, 160, 0, 40);

            // Blit side slice to last slice of screen
            Gfx::alt_blit(side_buffer, anim_x, 40, SLICE, 160, 240 - SLICE, 40);
            break;
        case ANIMATE_LEFT_TURN:
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
                Phantom &p = game.phantoms.at(0);
                Gfx::draw_screen(p.x, p.y, p.direction);
            } else if (map_mode) {
                // Draw an overhead view
                Map::draw(BASE_MAP_DELTA, true);
            } else {
                // Show the player's view
                Gfx::draw_screen(game.player.x, game.player.y, game.player.direction);
            }

            // Don't show gunnery if a Phantom has been hit
            if (game.state == ZAP_PHANTOM) return;

            // Is the laser being fired?
            if (game.is_firing) Gfx::draw_zap(game.zap_frame);

            // Has the player primed the laser?
            if (game.show_reticule) Gfx::draw_reticule();
    }
}


/*
 *      INITIALISATION FUNCTIONS
 */
static void setup_device(void) {

    // Set the LCD backlight
    // TODO Make this controllable during the game
    backlight(100);

    // Use one of the Pico's other analog inputs
    // to seed the random number generator
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);

    // Randomise using TinyMT
    // https://github.com/MersenneTwister-Lab/TinyMT
    tinymt32_init(&tinymt_store, adc_read() + 1 * battery());
    //std::srand(RANDSEED);

    // Make the graphic frame rects
    // NOTE These are pixel values:
    //      left, top, width, height, Phantom lateral offset
    uint8_t coords[] = { 0,    0, 240, 160, 60,     // Outer LED frame
                         20,  10, 200, 140, 50,
                         44,  20, 152, 120, 38,
                         66,  30, 108, 100, 26,
                         88,  40,  64,  80, 20,
                         102, 46,  36,  68,  8,
                         114, 50,  12,  58,  2};    // 'End wall' for distant views

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
    game.state = ANIMATE_LOGO;

    #ifdef DEBUG
    printf("DONE SETUP_DEVICE\n");
    #endif
}


/**
 * @brief Start a new game by re-initialising the game state,
 *        and setting up a new maze. Called at the start of the
 *        first game and subsequently when the player dies.
 */
static void start_new_game(void) {

    // Reset the settings
    init_game();
    init_phantoms();
    start_new_level();

    // Clear the screen (blue), present the current map
    // and give the player a five-second countdown before
    // entering the maze
    Gfx::cls(BLUE);
    Map::draw(BASE_MAP_DELTA, false);

    Gfx::draw_number(game.level, 156, 12, true);
    Gfx::draw_word(WORD_LEVEL, 72, 12, true);

    // Set the game mode
    game.state = START_COUNT;

    #ifdef DEBUG
    printf("DONE START_NEW_GAME()\n");
    #endif
}


/**
 * @brief Reset the main game control structure. Called only at the start of
 *        a game, not the start of a level (see `init_level()`).
 *
 *        NOTE Phantom data is separated out into `init_phantoms()`.
 */
static void init_game(void) {

    // If either of these demo/test modes are both set,
    // chase mode takes priority
    chase_mode = false;
    map_mode = false;

    // FROM 1.0.2
    // Store the current map number so it's not
    // used in the next game
    game.map = ERROR_CONDITION;
    game.phantom_count = 1;

    game.player.x = 0;
    game.player.y = 0;
    game.player.direction = DIRECTION_NORTH;

    game.tele_x = 0;
    game.tele_y = 0;
    game.start_x = 0;
    game.start_y = 0;

    game.level = 1;
    game.score = 0;
    game.kills = 0;

    game.crosshair_delta = 0;
    game.audio_range = 4;

    #ifdef DEBUG
    printf("DONE INIT_GAME()\n");
    #endif
}


/**
 * @brief Generate and populate a new level. This happens
 *        at the start of a new game and at the start of
 *        each level. A level jump is triggered when all the
 *        current phantoms have been dispatched.
 */
static void start_new_level(void) {

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
    game.player.direction = (uint8_t)(Utils::irandom(0, 4));
    //if (game.player.direction > 3) game.player.direction = 1
    game.start_x = x;
    game.start_y = y;

    #ifdef DEBUG
    printf("PLAYER LOCATION: %i, %i. DIRECTION: %i\n", x, y, game.player.direction);
    #endif

    // Set the teleport
    set_teleport_square();

    // Add the first phantom to the map, everywhere but empty
    // or where the player has been placed.
    // NOTE 'game.phantom_count' set in 'init_game()' and 'manage_phantoms()'
    for (uint8_t i = 0 ; i < game.phantom_count ; i++) {
        Phantom &p = game.phantoms.at(i);
        p.init();
        p.place(i);
    }

    /* TEST DATA
    game.player.x = 0;
    game.player.y = 0;
    game.player.direction = DIRECTION_EAST;
     */

    #ifdef DEBUG
    printf("DONE START_NEW_LEVEL()\n");
    #endif
}


/**
 * @brief Reset the main game control structure values
 *        for the start of a level.
 */
static void init_level(void) {

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
static void init_phantoms(void) {

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
 @brief Randomly roll a teleport square.
 */
static void set_teleport_square(void) {

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
static void update_world(void) {

    // Move the Phantom(s) periodically -- this is how
    // we increase their speed as the game progresses
    uint32_t now = time_us_32();
    if (now - game.last_phantom_move > game.phantom_speed) {
        game.last_phantom_move = now;
        if (!move_phantoms()) {
            check_senses();
        } else {
            // Player was killed
            death();

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
 * @brief Scan around the player for nearby Phantoms.
 */
static void check_senses(void) {

    int dx = game.player.x - game.audio_range;
    int dy = game.player.y - game.audio_range;

    for (int x = dx ; x < (dx + (game.audio_range << 1)) ; x++) {
        if (x < 0) continue;
        if (x > MAP_MAX) break;
        for (int y = dy ; y < (dy + (game.audio_range << 1)) ; y++) {
            if (y < 0) continue;
            if (y > MAP_MAX) break;
            uint8_t nabbed = Map::phantom_on_square(x, y);
            if (nabbed != ERROR_CONDITION) {
                // There's a Phantom in range, so sound a tone
                led(100, 0, 0);
                Utils::beep();
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
 * @brief Check whether we need to increase the number of phantoms
 *        on the board or increase their speed -- all caused by a
 *        level-up. We up the level if all the level's phantoms have
 *        been zapped.
 */
static void manage_phantoms(void) {

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
        for (uint8_t i = 0 ; i < MAX_PHANTOMS ; i++) {
            Phantom &p = game.phantoms.at(i);
            p.x = ERROR_CONDITION;
            p.y = ERROR_CONDITION;
        }

        // Create a new level
        start_new_level();
    }
}


/**
 * @brief Tell all of the current Phantoms to move.
 *
 * @returns `true` if a Phantom caught the Player,
            otherwise `false`.
*/
static bool move_phantoms(void) {

    for (uint8_t i = 0 ; i < MAX_PHANTOMS ; i++) {
        Phantom &p = game.phantoms.at(i);
        if (p.move()) return true;
    }

    return false;
}


/**
 * @brief Return the index of the closest facing Phantom to the
 *        player from the the 'game.phantoms' vector --
 *        or `ERROR_CONDITION`.
 *
 * @param range: The number of squares to iterate over.
 *
 * @returns The index of the Phantom in the vector.
 */
static uint8_t get_facing_phantom(uint8_t range) {

    uint8_t p_index = ERROR_CONDITION;
    switch(game.player.direction) {
        case DIRECTION_NORTH:
            if (game.player.y == 0) return ERROR_CONDITION;
            if (game.player.y - range < 0) range = game.player.y;
            for (int8_t i = game.player.y ; i > game.player.y - range ; --i) {
                p_index = Map::phantom_on_square(game.player.x, i);
                if (p_index != ERROR_CONDITION) return p_index;
            }
            break;
        case DIRECTION_EAST:
            if (game.player.x == MAP_MAX) return ERROR_CONDITION;
            if (game.player.x + range > MAP_MAX) range = MAP_MAX - game.player.x;
            for (int8_t i = game.player.x ; i < game.player.x + range ; ++i) {
                p_index = Map::phantom_on_square(i, game.player.y);
                if (p_index != ERROR_CONDITION) return p_index;
            }
            break;
        case DIRECTION_SOUTH:
            if (game.player.y == MAP_MAX) return ERROR_CONDITION;
            if (game.player.y + range > MAP_MAX) range = MAP_MAX - game.player.y;
            for (int8_t i = game.player.y ; i < game.player.y + range ; ++i) {
                p_index = Map::phantom_on_square(game.player.x, i);
                if (p_index != ERROR_CONDITION) return p_index;
            }
            break;
        default:
            if (game.player.x == 0) return ERROR_CONDITION;
            if (game.player.x - range < 0) range = game.player.x;
            for (int8_t i = game.player.x ; i > game.player.x - range ; --i) {
                p_index = Map::phantom_on_square(i, game.player.y);
                if (p_index != ERROR_CONDITION) return p_index;
            }
    }

    return p_index;
}


/**
 * @brief Return the number of Phantoms in front of the player.
 *
 * @param range: The number of squares to iterate over.
 *
 * @returns The number of Phantoms in front of the Player.
 */
uint8_t count_facing_phantoms(uint8_t range) {

    uint8_t phantom_count = 0;
    switch(game.player.direction) {
        case DIRECTION_NORTH:
            if (game.player.y == 0) return phantom_count;
            if (game.player.y - range < 0) range = game.player.y;
            for (int8_t i = game.player.y ; i >= game.player.y - range ; --i) {
                phantom_count += (Map::phantom_on_square(game.player.x, i) != ERROR_CONDITION ? 1 : 0);
            }
            break;
        case DIRECTION_EAST:
            if (game.player.x == MAP_MAX) {
                Gfx::draw_number(8, 90, 12);
                return phantom_count;
            }
            if (game.player.x + range > MAP_MAX) range = MAP_MAX - game.player.x;
            for (int8_t i = game.player.x ; i <= game.player.x + range ; ++i) {
                phantom_count += (Map::phantom_on_square((uint8_t)i, game.player.y) != ERROR_CONDITION ? 1 : 0);
            }
            break;
        case DIRECTION_SOUTH:
            if (game.player.y == MAP_MAX) return phantom_count;
            if (game.player.y + range > MAP_MAX) range = MAP_MAX - game.player.y;
            for (int8_t i = game.player.y ; i <= game.player.y + range ; ++i) {
                phantom_count += (Map::phantom_on_square(game.player.x, i) != ERROR_CONDITION ? 1 : 0);
            }
            break;
        default:
            if (game.player.x == 0) return phantom_count;
            if (game.player.x - range < 0) range = game.player.x;
            for (int8_t i = game.player.x ; i >= game.player.x - range ; --i) {
                phantom_count += (Map::phantom_on_square(i, game.player.y) != ERROR_CONDITION ? 1 : 0);
            }
    }

    Gfx::draw_number(phantom_count, 80, 12);
    return phantom_count;
}


/**
 * @brief Get player movement action from the joypad
 *        Favour movement over rotation.
 *
 * @param keys_pressed: Bitfield indicating which
 *                      keys have been pressed.
 *
 * @returns The direction in which the player is facing.
 */
static uint8_t get_direction(uint8_t keys_pressed) {

    if (keys_pressed & 0x10) return MOVE_FORWARD;
    if (keys_pressed & 0x20) return MOVE_BACKWARD;
    if (keys_pressed & 0x40) return TURN_LEFT;
    if (keys_pressed & 0x80) return TURN_RIGHT;

    // Just in case
    return ERROR_CONDITION;
}


/*
 * ACTIONS
 */

/**
 * @brief Jump back to the teleport square if the player has walked over it.
 */
static void do_teleport(void) {

    // Move the player to the stored square
    game.state = DO_TELEPORT_ONE;
    tele_flash_time = time_us_32();

    // Reset the laser if it's firing
    reset_laser();

    // Re-locate the teleport square
    set_teleport_square();
}


/**
    Hit the front-most facing Phantom, if there is one.
 */
static void fire_laser(void) {

    // Did we hit a Phantom?
    play(zap, 640, 200);
    uint8_t n = get_facing_phantom(MAX_VIEW_RANGE);
    if (n != ERROR_CONDITION) {
        // A hit! A palpable hit!
        // Deduct 1HP from the Phantom
        Phantom &p = game.phantoms.at(n);
        p.hp--;

        // FROM 1.0.2
        // Use original scoring: 2 points for a hit, 10 for a kill
        game.score += 2;
        game.level_hits++;
        game.is_firing = false;

        // Did that kill it?
        if (p.hp < 1) {
            // Yes! One dead Phantom...
            game.score += 10;
            game.level_kills++;
            game.kills++;

            // Briefly invert the screen and sound some tones
            // tone(1200, 100, 200);
            // tone(600, 100, 200);

            // Quickly show the map
            game.state = ZAP_PHANTOM;
            dead_phantom = n;

            // Reset the laser
            reset_laser();
        }
    }
}


/**
 * @brief Reset the laser after firing.
 */
static void reset_laser(void) {

    game.is_firing = false;
    game.can_fire = false;
    game.zap_charge_time = time_us_32();
    game.zap_frame = 0;
}


/*
 * GAME OUTCOMES
 */

/**
 * @bried The player has died -- show the map and the score.
 */
static void death(void) {

    game.state = PLAYER_IS_DEAD;
    for (unsigned int i = 0 ; i < 4 ; i++) {
        play(roar, 150, 500, 100);
        sleep(1000);
    }

    // Clear the display (blue)
    Gfx::cls(BLUE);

    // Give instructions
    //Gfx::draw_word(PHRASE_ANY_KEY, 44, 215, true);

    // Show the map
    show_scores(true);

    // Clear a space and say what happened
    //pen(BLUE);
    //frect(36, 106, 168, 28);
    //Gfx::draw_word(PHRASE_PLAYER_DEAD, 38, 110, true);
}


/**
 * @brief Just show the map briefly after killing a Phantom.
 * 
 * @param is_last: The phantom is the last one on the level.
 */
static void phantom_killed(bool is_last) {

    Gfx::cls(BLUE);
    show_scores(is_last);
}


/**
 * @brief Code used in a couple of 'show map' locations
 *        and show the current score alongside the map.
 */
static void show_scores(bool show_tele) {

    uint8_t cx = 10;
    if (game.high_score < game.score) game.high_score = game.score;

    // Show the score
    Gfx::draw_word(WORD_SCORE, 10, 5, false);
    uint32_t score = Utils::bcd(game.score);
    Gfx::draw_number((score & 0xF000) >> 12, cx, 18, true);
    cx += (((score & 0xF000) >> 12) == 1 ? 6 : 14);
    Gfx::draw_number((score & 0x0F00) >> 8, cx, 18, true);
    cx += (((score & 0x0F00) >> 8) == 1 ? 6 : 14);
    Gfx::draw_number((score & 0x00F0) >> 4, cx, 18, true);
    cx += (((score & 0x00F0) >> 4) == 1 ? 6 : 14);
    Gfx::draw_number(score & 0x000F, cx, 18, true);

    // Show the high score
    Gfx::draw_word(WORD_HIGH, 162, 5, false);
    Gfx::draw_word(WORD_SCORE, 192, 5, false);
    score = Utils::bcd(game.high_score);
    cx = (score & 0x000F == 1) ? 226 : 218;
    Gfx::draw_number(score & 0x000F, cx, 18, true);
    cx = Utils::fix_num_width((score & 0x00F0) >> 4, cx);
    Gfx::draw_number((score & 0x00F0) >> 4,  cx, 18, true);
    cx = Utils::fix_num_width((score & 0x0F00) >> 8, cx);
    Gfx::draw_number((score & 0x0F00) >> 8,  cx, 18, true);
    cx = Utils::fix_num_width((score & 0xF000) >> 12, cx);
    Gfx::draw_number((score & 0xF000) >> 12, cx, 18, true);

    if (game.state != PLAYER_IS_DEAD) {
        // This is for the intermediate map only
        // Show kills
        Gfx::draw_word(WORD_KILLS, 198, 228, false);
        score = Utils::bcd(game.level_kills);
        cx = (score == 1) ? 226 : 220;
        Gfx::draw_number(score, cx, 204, true);

        // Show hits
        Gfx::draw_word(WORD_HITS, 10, 228, false);
        score = Utils::bcd(game.level_hits);
        Gfx::draw_number(score, 10, 204, true);
    }

    // Add in the map
    Map::draw(BASE_MAP_DELTA, true, show_tele);
}

