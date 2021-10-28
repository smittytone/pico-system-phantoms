/*
 * phantom-slayer for Raspberry Pi Pico
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#include "main.h"

using namespace picosystem;


/*
 *  GLOBALS
 */
uint8_t     count_down = 5;
uint8_t     dead_phantom = ERROR_CONDITION;
uint8_t     help_page_count = 0;

bool        chase_mode = false;
bool        map_mode = false;
bool        tele_state = false;

uint32_t    tele_flash_time = 0;
uint32_t    tick_count = 0;

tinymt32_t  tinymt_store;

Rect        rects[7];

Game        game;

voice_t blip = voice(10, 10, 10, 10, 40, 2);



/*
 *  PICOSYSTEM CALLBACKS
 */
void init() {
    // Clear the display as soon as possible
    pen(0, 15, 0);
    clear();

    #ifdef DEBUG
    // Enable debugging
    stdio_init_all();
    #endif

    // Set up the game
    setup();

    // Display the intro animation
    // play_intro();

    // Start a new game -- the first
    start_new_game();

    #ifdef DEBUG
    printf("INIT() DONE\n");
    #endif
}


void update(uint32_t tick_ms) {
    uint32_t now = time_us_32();
    switch (game.state) {
        case PLAY_INTRO:
            break;
        case SHOW_HELP:
            // Run through the help pages with
            // eack key press
            if (Utils::inkey() > 0) help_page_count++;
            if (help_page_count > MAX_HELP_PAGES) start_new_game();
            break;
        case START_COUNT:
            // Count down five seconds before
            // actually starting the game
            tick_count++;
            if (tick_count % 100 == 0) count_down--;
            if (count_down == 0) {
                tick_count = 0;
                game.state = IN_PLAY;
            }
            break;
        case PLAYER_IS_DEAD:
            // NOTE Call 'death()' before coming here
            // Just await any key press to start again
            if (Utils::inkey() > 0) start_new_game();
            break;
        case DO_TELEPORT_ONE:
            // Flip between TELE_ONE and TELE_TWO
            // every 1/4 second for two seconds
            tick_count++;
            if (tick_count == 25) {
                tick_count = 0;
                game.state = DO_TELEPORT_TWO;
            }
            break;
        case DO_TELEPORT_TWO:
            tick_count++;
            if (tick_count == 25) {
                tick_count = 0;
                game.state = now - tele_flash_time < 2000000 ? DO_TELEPORT_ONE : IN_PLAY;
            }
            break;
        case SHOW_TEMP_MAP:
            // Wait 3s while the post-kill
            // map is on screen
            tick_count++;
            if (tick_count == 300) {
                tick_count = 0;
                game.state = IN_PLAY;
            }
            break;
        case ZAP_PHANTOM:
            tick_count++;
            if (tick_count == 100) {
                tick_count = 0;
                game.state = SHOW_TEMP_MAP;

                // Take the dead phantom off the board
                // (so it gets re-rolled in 'manage_phantoms()')
                game.phantoms.erase(game.phantoms.begin() + dead_phantom);
                manage_phantoms();
                dead_phantom = ERROR_CONDITION;
            }
        default:
            // The game is afoot! game.state = IN_PLAY
            // NOTE Return as quickly as possible

            // Was a key tapped?
            uint8_t key = Utils::inkey();

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
                        #ifdef DEBUG
                        printf("%i,%i\n", nx, ny);
                        #endif

                        // Has the player walked up to a Phantom?
                        if (Map::phantom_on_square(nx, ny) != ERROR_CONDITION) {
                            // Yes -- so the player is dead!
                            death();
                            game.state = PLAYER_IS_DEAD;

                            #ifdef DEBUG
                            printf("\nPLAYER IS DEAD\n");
                            #endif
                            return;
                        }

                        // Set the new square for rendering later
                        game.player.x = nx;
                        game.player.y = ny;
                    }
                } else if (dir == TURN_RIGHT) {
                    // Turn player right
                    game.player.direction++;
                    if (game.player.direction > DIRECTION_WEST) game.player.direction = DIRECTION_NORTH;

                    // Animate the turn now
                    //if (!chase_mode && !map_mode) Gfx::animate_turn(false);
                } else if (dir == TURN_LEFT) {
                    // Turn player left
                    game.player.direction--;
                    if (game.player.direction > DIRECTION_WEST) game.player.direction = DIRECTION_WEST;

                    // Animate the turn now
                    //if (!chase_mode && !map_mode) Gfx::animate_turn(true);
                }
            } else if ((key & 0x02) && !game.show_reticule) {
                // Player can only teleport if they have walked over the
                // teleport square and they are not firing the laser
                if (game.player.x == game.tele_x && game.player.y == game.tele_y) {
                    do_teleport();
                }
            } else if (key & 0x04) {
                // Map mode should be for debugging only
                map_mode = !map_mode;
            } else if (key & 0x08) {
                // Lower radar range
                game.audio_range--;
                if (game.audio_range < 1) game.audio_range = 6;
            }

            // Check for firing
            // NOTE This uses separate code because it requires the button
            //      to be held down (fire on release)
            if (button(A)) {
                if (game.can_fire) {
                    // Button A pressed
                    if (!game.show_reticule) {
                        game.show_reticule = true;
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
                }
            }

            // Manage the world
            update_world();
    }
}


void draw() {
    uint8_t nx;
    switch(game.state) {
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
            pen(0, 0, 15);
            frect(221, 40, 14, 21);

            // Show the new number
            pen(15, 15, 0);
            nx = (count_down == 1 ? 225 : 221);
            Gfx::draw_number(count_down, nx, 40, true);
            break;
        case SHOW_TEMP_MAP:
            // We've already drawn the post kill map, so just exit
        case PLAYER_IS_DEAD:
            // We've already drawn the end-of-game map, so just exit
            break;
        default:
            // game.state == IN_PLAY
            // Render the screen
            if (chase_mode) {
                // Show the first Phantom's view
                Phantom &p = game.phantoms.at(0);
                Gfx::draw_screen(p.x, p.y, p.direction);
            } else if (map_mode) {
                // Draw an overhead view
                Map::draw(0, true);
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
void setup() {
    // Use one of the Pico's other analog inputs
    // to seed the random number generator
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);
    srand(picosystem::battery() * 100);

    // Randomise using TinyMT
    // https://github.com/MersenneTwister-Lab/TinyMT
    tinymt32_init(&tinymt_store, adc_read());

    // Make the graphic frame rects
    // NOTE These are pixel values:
    //      left, top, width, height, Phantom lateral offset
    uint8_t coords[] = { 0,    0, 240, 160, 60,     // Outer LED frame
                         20,  10, 200, 140, 50,
                         44,  20, 152, 120, 38,
                         66,  30, 108, 100, 26,
                         88,  40,  64,  80, 20,
                         102, 46,  36,  68,  8,
                         114, 50,  12,  60,  2};    // 'End wall' for distant views

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

    // Set the LCD backlight
    backlight(90);
}


/*
    Start a new game by re-initialising the game state,
    and setting up a new maze.
 */
void start_new_game() {
    // Reset the settings and roll the world
    init_game();
    init_phantoms();
    create_world();

    // Clear the screen, present the current map and
    // give the player a five-second countdown
    pen(0, 0, 15);
    clear();
    Map::draw(0, false);

    Gfx::draw_word(WORD_NEW, 0, 40);
    Gfx::draw_word(WORD_GAME, 0, 52);

    Gfx::draw_word(WORD_LEVEL, 0, 168);
    Gfx::draw_number(1, 18, 180, true);

    Gfx::draw_number(5, 223, 40, true);

    // Set the loop mode
    game.state = START_COUNT;
}


/*
    Reset the main game control structure. Called only at the start of
    a game, not the start of a level.

    NOTE Phantom data is separated out into `init_phantoms()`.
 */
void init_game() {
    // If these demo/test modes are both set,
    // chase mode takes priority
    chase_mode = false;
    map_mode = false;

    // FROM 1.0.2
    // Store the current map number so it's not
    // used in the next game
    game.map = ERROR_CONDITION;

    game.player.x = 0;
    game.player.y = 0;
    game.player.direction = 0;

    game.show_reticule = false;
    game.can_fire = true;
    game.is_firing = false;

    game.state = PLAY_INTRO;
    game.tele_x = 0;
    game.tele_y = 0;
    game.start_x = 0;
    game.start_y = 0;

    game.level = 0;
    game.score = 0;
    game.kills = 0;
    game.level_kills = 0;
    game.level_hits = 0;

    game.zap_charge_time = 0;
    game.zap_fire_time = 0;
    game.zap_frame = 0;

    #ifdef DEBUG
    printf("DONE INIT_GAME\n");
    #endif
}


/*
    Initialise the current game's Phantom data.
 */
void init_phantoms() {
    // Reset the array stored phantoms structures
    game.phantoms.clear();
    game.phantom_speed = PHANTOM_MOVE_TIME_US << 1;
    game.last_phantom_move = 0;
}


/**
    Generate and populate a new level. This happens
    at the start of a new game and at the start of
    each level. A level jump is triggered when all the
    current phantoms have been dispatched.
 */
void create_world() {
    // Reset the game
    game.level++;

    // Initialise the current map
    game.map = Map::init(game.map);

    // Set the teleport
    while (true) {
        // Pick a random co-ordinate
        uint8_t x = Utils::irandom(0, 20);
        uint8_t y = Utils::irandom(0, 20);

        if (Map::get_square_contents(x, y) == MAP_TILE_CLEAR) {
            game.tele_x = x;
            game.tele_y = y;
            break;
        }
    }

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
    game.player.direction = Utils::irandom(0, 4);
    game.start_x = x;
    game.start_y = y;

    // Reset the the phantoms data
    init_phantoms();

    // Add the first phantom to the map, everywhere but empty
    // or where the player
    Phantom p = Phantom(0,0);
    while (true) {
        // Pick a random co-ordinate
        uint8_t x = Utils::irandom(0, 20);
        uint8_t y = Utils::irandom(0, 20);

        // If the chosen square is valid, use it
        if ((x < game.player.x - 4 || x > game.player.x + 4) && (y < game.player.y - 4 || y > game.player.y + 4)) {
            if (Map::get_square_contents(x, y) == MAP_TILE_CLEAR) {
                p.x = x;
                p.y = y;
                break;
            }
        }
    }

    game.phantoms.push_back(p);

    /* TEST DATA
    game.player.x = 0;
    game.player.y = 0;
    game.player.direction = DIRECTION_EAST;
     */

    #ifdef DEBUG
    printf("DONE CREATE_WORLD\n");
    #endif
}


/**
    Update the world at the end of the move cycle.

    Called from the main `update()` callback.
 */
void update_world() {
    // Move the Phantoms periodically -- this is how
    // we increase their speed as the game progresses
    uint32_t now = time_us_32();
    if (now - game.last_phantom_move > game.phantom_speed) {
        game.last_phantom_move = now;
        if (!move_phantoms()) {
            check_senses();
        } else {
            // Player was killed
            game.state = PLAYER_IS_DEAD;
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
    Get player movement action from the joypad
    Favour movement over rotation.

    - Parameters:
        - keys_pressed: Bitfield indicating which
                        keys have been pressed.

    - Returns: The direction in which the player is facing.
 */
uint8_t get_direction(uint8_t keys_pressed) {
    if (keys_pressed & 0x10) return MOVE_FORWARD;
    if (keys_pressed & 0x20) return MOVE_BACKWARD;
    if (keys_pressed & 0x40) return TURN_LEFT;
    if (keys_pressed & 0x80) return TURN_RIGHT;

    // Just in case
    return ERROR_CONDITION;
}


/**
    Return the index of the closest facing Phantom to the
    player from the the 'game.phantoms' vector --
    or `ERROR_CONDITION`.

    - Parameters:
        - range: The number of squares to iterate over.

    - Returns: The index of the Phantom in the vector.
 */
uint8_t get_facing_phantom(uint8_t range) {
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
    Return the number of Phantoms in front of the player.

    - Parameters:
        - range: The number of squares to iterate over.

    - Returns: The number of Phantoms in front of the Player.
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
    Tell all of the current Phantoms to move.
*/
bool move_phantoms() {
    size_t number = game.phantoms.size();
    if (number > 0) {
        for (size_t i = 0 ; i < number ; ++i) {
            Phantom &p = game.phantoms.at(i);
            if (p.move() == true) return true;

            #ifdef DEBUG
            printf("Moving Phantom %i of %i\n", i, number);
            #endif
        }
    }

    return false;
}


void beep() {
    play(blip, 1800, 30, 100);
}


/*
 *      ACTIONS
 */

/**
    Scan around the player for nearby Phantoms.
 */
void check_senses() {
    int8_t dx = game.player.x - game.audio_range;
    int8_t dy = game.player.y - game.audio_range;

    for (int8_t i = dx ; i < dx + (game.audio_range << 1) ; ++i) {
        if (i < 0) continue;
        if (i > MAP_MAX) break;
        for (int8_t j = dy ; j < dy + (game.audio_range << 1) ; ++j) {
            if (j < 0) continue;
            if (j > MAP_MAX) break;
            if (Map::phantom_on_square(i, j) != ERROR_CONDITION) {
                // There's a Phantom in range, so sound a tone
                beep();

                // Only play one beep, no matter
                // how many nearby phantoms there are
                return;
            }
        }
    }
}


/**
    Jump back to the teleport square if the player has walked over it.
 */
void do_teleport() {
    // Move the player to the stored square
    game.player.x = game.start_x;
    game.player.y = game.start_y;
    game.state = DO_TELEPORT_ONE;
    tele_flash_time = time_us_32();

    // Reset the laser if firing
    reset_laser();
}


/**
    Hit the front-most facing Phantom, if there is one.
 */
void fire_laser() {
    // Did we hit a Phantom?
    uint8_t n = get_facing_phantom(MAX_VIEW_RANGE);
    if (n != ERROR_CONDITION) {
        // A hit! A palpable hit!
        // Deduct 1HP from the Phantom
        Phantom &p = game.phantoms.at(n);
        p.hp -= 1;

        // FROM 1.0.2
        // Use original scoring: 2 points for a hit, 10 for a kill
        game.score += 2;
        game.level_hits++;
        game.is_firing = false;

        // Did that kill it?
        if (p.hp == 0) {
            // Yes! One dead Phantom...
            game.score += 8;
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

            // Show the map
            phantom_killed();
        }

        // Update phantoms list
        //manage_phantoms();
    }
}


void reset_laser() {
    game.is_firing = false;
    game.can_fire = false;
    game.zap_charge_time = time_us_32();
    game.zap_frame = 0;
}


/**
    Check whether we need to increase the number of phantoms
    on the board or increase their speed -- all caused by a
    level-up. We up the level if all the level's phantoms have
    been zapped.
 */
void manage_phantoms() {
    bool level_up = false;
    size_t phantom_count = 0;

    // If we're on levels 1 and 2, we only have that number of
    // Phantoms. From 3 and up, there are aways three in the maze
    if (game.level < MAX_PHANTOMS) {
        if (game.level_kills == game.level) {
            game.level_kills = 0;
            game.level_hits = 0;
            game.level++;
            level_up = true;
            phantom_count = game.level;
        }
    } else {
        if (game.level_kills == MAX_PHANTOMS) {
            game.level_kills = 0;
            game.level_hits = 0;
            level_up = true;
            game.level++;
            phantom_count = MAX_PHANTOMS;
        }
    }

    // Did we level-up? Is so, update the phantom movement speed
    if (level_up) {
        uint8_t index = (game.level - 1) * 4;
        game.phantom_speed = ((PHANTOM_MOVE_TIME_US << level_data[index + 2]) >> level_data[index + 3]);

        // Just in case...
        if (phantom_count > MAX_PHANTOMS) phantom_count = MAX_PHANTOMS;

        // Do we need to add any new phantoms to the board?
        while (game.phantoms.size() < phantom_count) {
            Phantom p = Phantom();
            p.roll_location();
            game.phantoms.push_back(p);
        }
    }

    #ifdef DEBUG
    printf("Phantoms: %i\n", game.phantoms.size());
    #endif
}


/*
 *      GAME OUTCOMES
 */

/**
    The player has died -- show the map and the score.
 */
void death() {
    //for (unsigned int i = 400 ; i > 100 ; i -= 2) tone(i, 30, 0);
    //sleep_ms(50);
    //tone(2200, 500, 600);

    // Clear the display
    pen(0, 0, 15);
    clear();

    // Give instructions
    //Gfx::draw_word(PHRASE_ANY_KEY, 10, 220);

    // Show the map
    show_scores();
}


/**
    Just show the map briefly after killing a Phantom
 */
void phantom_killed() {
    pen(0, 0, 15);
    clear();
    show_scores();
}


/**
    Code used in a couple of 'show map' locations
    Show the current score alongside the map.
 */
void show_scores() {
    uint8_t cx = 10;
    if (game.high_score < game.score) game.high_score = game.score;

    // Show the score
    Gfx::draw_word(WORD_SCORE, 10, 5);
    uint32_t score = Utils::bcd(game.score);
    Gfx::draw_number((score & 0xF000) >> 12, cx, 18, true);
    cx += (((score & 0xF000) >> 12) == 1 ? 6 : 14);
    Gfx::draw_number((score & 0x0F00) >> 8, cx, 18, true);
    cx += (((score & 0x0F00) >> 8) == 1 ? 6 : 14);
    Gfx::draw_number((score & 0x00F0) >> 4, cx, 18, true);
    cx += (((score & 0x00F0) >> 4) == 1 ? 6 : 14);
    Gfx::draw_number(score & 0x000F, cx, 18, true);

    // Show the high score
    Gfx::draw_word(WORD_HIGH, 162, 5);
    Gfx::draw_word(WORD_SCORE, 192, 5);
    score = Utils::bcd(game.high_score);
    cx = (score & 0x000F == 1) ? 226 : 218;
    Gfx::draw_number(score & 0x000F, cx, 18, true);
    cx = fix_num_width((score & 0x00F0) >> 4, cx);
    Gfx::draw_number((score & 0x00F0) >> 4,  cx, 18, true);
    cx = fix_num_width((score & 0x0F00) >> 8, cx);
    Gfx::draw_number((score & 0x0F00) >> 8,  cx, 18, true);
    cx = fix_num_width((score & 0xF000) >> 12, cx);
    Gfx::draw_number((score & 0xF000) >> 12, cx, 18, true);

    if (game.state != PLAYER_IS_DEAD) {
        // This is for the intermediate map only
        // Show kills
        Gfx::draw_word(WORD_KILLS, 198, 225);
        score = Utils::bcd(game.level_kills);
        cx = (score == 1) ? 190 : 182;
        Gfx::draw_number(score, cx, 215, true);

        // Show hits
        Gfx::draw_word(WORD_HITS, 10, 225);
        score = Utils::bcd(game.level_hits);
        Gfx::draw_number(score, 39, 215, true);
    }

    // Add in the map
    Map::draw(4, true);
}


uint8_t fix_num_width(uint8_t value, uint8_t current) {
    return (current - (value == 1 ? 6 : 14));
}