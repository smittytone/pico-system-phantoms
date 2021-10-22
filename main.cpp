/*
 * phantom-slayer for Raspberry Pi Pico
 *
 * @version     1.0.0
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
uint8_t count = 0;

tinymt32_t  tinymt_store;

bool        chase_mode = false;
bool        map_mode = false;;

// Game data
Game     game;

// Graphics structures
Rect        rects[7];

extern const uint8_t level_data[84];


/*
 *  PICOSYSTEM CALLBACKS
 */
void init() {
    // Use for debugging
    stdio_init_all();

    // Set up the game
    setup();

    // Display the intro animation
    // play_intro();

    // Start a new game -- the first
    start_new_game();

    printf("DONE INIT\n");
}


void update(uint32_t tick_ms) {

    switch (game.state) {
        case PLAYER_IS_DEAD:
            // Just await any key press to start again
            if (Utils::inkey() > 0) {
                // Start a new game
                start_new_game();
            }
            break;
        case START_COUNT:
            // Count down five seconds
            if (tick_ms % 100 == 0) {
                count++;
                // bleep
                // Update on screen number
            }

            if (count == 0) {
                game.state = IN_PLAY;
            }
            break;
        default:
            // The game is afoot! game.state = IN_PLAY
            // NOTE Return as quickly as possible

            // Was a key tapped?
            uint8_t key = Utils::inkey();

            if (key > 0x0F) {
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
                        printf("%i,%i\n",nx,ny);
                        if (Map::phantom_on_square(nx, ny) != ERROR_CONDITION) {
                            // Yes -- so the player is dead!
                            game.state = PLAYER_IS_DEAD;
                            printf("DEAD\n");
                            return;
                        }

                        // Set the new square for rendering later
                        game.player.x = nx;
                        game.player.y = ny;

                    }
                } else if (dir == TURN_RIGHT) {
                    // Turn player right
                    ++game.player.direction;
                    if (game.player.direction > DIRECTION_WEST) game.player.direction = DIRECTION_NORTH;

                    // Animate the turn now
                    //if (!chase_mode && !map_mode) Gfx::animate_turn(false);
                } else if (dir == TURN_LEFT) {
                    // Turn player left
                    --game.player.direction;
                    if (game.player.direction > DIRECTION_WEST) game.player.direction = DIRECTION_WEST;

                    // Animate the turn now
                    //if (!chase_mode && !map_mode) Gfx::animate_turn(true);
                }
            } else if (key & 0x02) {
                // Player can only teleport if they have walked over the
                // teleport square and they are not firing the laser
                if (!game.show_reticule) {
                    // Teleport if the player's the square
                    if (game.player.x == game.tele_x && game.player.y == game.tele_y) {
                        do_teleport();
                    }
                }
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
                    game.is_firing = true;
                    game.can_fire = false;
                    game.zap_charge_time = time_us_32();
                }
            }
    }

    // Manage and draw the world
    update_world();
}


void draw() {

    if (game.state == IN_PLAY) {
        // Render the screen
        if (chase_mode) {
            // Show the first Phantom's view
            Gfx::draw_screen(game.phantoms[0].x, game.phantoms[0].y, game.phantoms[0].direction);
        } else if (map_mode) {
            // Draw the map
            Map::draw(0, true);
        } else {
            // Show the player's view
            Gfx::draw_screen(game.player.x, game.player.y, game.player.direction);
        }

        // Is the laser being fired?
        if (game.is_firing) {
            Gfx::draw_zap();
        }

        // Has the player primed the laser? If so show the crosshair
        if (game.show_reticule) {
            Gfx::draw_reticule();
        }
    }
}


/*
 *      INITIALISATION FUNCTIONS
 */
void setup() {
    // Randomise using TinyMT
    // https://github.com/MersenneTwister-Lab/TinyMT
    //tinymt32_init(&tinymt_store, adc_read());
    // NOTE Need a entropy source for this.

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

    backlight(90);
}


/*
    Start a new game by re-initialising the game state,
    and setting up a new maze.
 */
void start_new_game() {
    init_game();
    create_world();
}


/*
    Reset the main game control structure.
    NOTE Phantom data is separated out into `init_phantoms()`.
 */
void init_game() {
    game.state = NOT_IN_PLAY;
    game.show_reticule = false;
    game.can_teleport = false;
    game.is_firing = false;
    game.can_fire = true;
    game.zap_frame = 0;
    game.zap_charge_time = 0;
    game.zap_fire_time = 0;
    game.level_score = 0;
    game.audio_range = 4;
    game.level = 1;
    game.tele_x = 0;
    game.tele_y = 0;
    game.start_x = 0;
    game.start_y = 0;

    // If these demo/test modes are both set,
    // chase mode takes priority
    chase_mode = false;
    map_mode = false;

    // FROM 1.0.2
    // Store the current map number so it's not
    // used in the next game
    game.map = ERROR_CONDITION;
    printf("DONE INIT_GAME\n");
}


/*
    Initialise the current game's Phantom data.
 */
void init_phantoms() {
    // Reset the array stored phantoms structures
    game.phantoms.clear();
    game.phantom_count = 0;
    game.phantom_speed = PHANTOM_MOVE_TIME_US << 1;
}


/*
    Generate and populate a new maze which happens
    at the start of a new game and at the start of
    each level. A level jump is triggered when all the
    current phantoms have been dispatched.
 */
void create_world() {
    // Reset the game
    if (game.level > 0) init_game();
    game.state = IN_PLAY;

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
    Phantom p = Phantom(x, y);
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

    // game.phantoms.push_back(p);


    game.player.x = 0;
    game.player.y = 0;
    game.player.direction = DIRECTION_NORTH;

    /* TEST DATA
    game.phantoms = 3;
    phantoms[0].x = 8;
    phantoms[0].y = 0;

    phantoms[1].x = 9;
    phantoms[1].y = 0;

    phantoms[2].x = 11;
    phantoms[2].y = 0;
     */

    printf("DONE CREATE_WORLD\n");
}


/*
    Update the world at the end of the move cycle.
 */
void update_world() {
    // Move the Phantoms periodically -- this is how
    // we increase their speed as the game progresses
    uint32_t now = time_us_32();
    if (now - game.last_phantom_move > game.phantom_speed) {
        game.last_phantom_move = now;
        move_phantoms();
        check_senses();
    }

    // Check for a laser recharge
    if (now - game.zap_charge_time > LASER_RECHARGE_US) {
        game.zap_charge_time = 0;
        game.can_fire = true;
    }

    // Animate the laser zap
    if (game.is_firing && now - game.zap_fire_time > LASER_FIRE_US) {
        game.zap_fire_time = now;
        game.zap_frame++;
        if (game.zap_frame == 6) {
            game.zap_frame = 0;
            game.is_firing = false;
        } else {
            fire_laser();
        }
    }
}


/*
    Tell all of the current Phantoms to move.
*/
void move_phantoms() {
    if (game.phantom_count > 0) {
        for (uint8_t i = 0 ; i < game.phantom_count ; ++i) {
            game.phantoms[i].move();
        }
    }
}


/*
 *      ACTIONS
 */

/*
    Scan around the player for nearby phantoms
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
            if (Map::phantom_on_square(i, j)) {
                // There's a Phantom in range, so
                // flash the LED and sound a tone
                // tone(200, 10, 0);

                // Only play one beep, no matter
                // how many nearby phantoms there are
                return;
            }
        }
    }
}


/*
    Jump back to the teleport square if the player has walked over it.
 */
void do_teleport() {
    // Flash the screen
    bool tstate = false;
    for (uint8_t i = 0 ; i < 10 ; ++i) {
        //tone((tstate ? 4000 : 2000), 100, 0);
        tstate = !tstate;
    }

    // Move the player to the stored square
    game.player.x = game.start_x;
    game.player.y = game.start_y;
}


/*
    Hit the front-most facing phantom, if there is one.
 */
void fire_laser() {
    // Did we hit a Phantom?
    uint8_t n = get_facing_phantom(MAX_VIEW_RANGE);
    if (n != ERROR_CONDITION) {
        // A hit! A palpable hit!
        // Deduct 1HP from the Phantom
        Phantom p = game.phantoms[n];
        p.hp -= 1;

        // FROM 1.0.2
        // Use original scoring: 2 points for a hit, 10 for a kill
        game.level_score += 2;

        // Did that kill it?
        if (p.hp == 0) {
            // Yes! One dead Phantom...
            game.level_score += 10;
            ++game.level_kills;

            // Briefly invert the screen and sound some tones
            // tone(1200, 100, 200);
            // tone(600, 100, 200);

            // Quickly show the map
            //ssd1306_clear();
            //show_scores();
            sleep_ms(MAP_POST_KILL_SHOW_MS);

            // Take the dead phantom off the board
            // (so it gets re-rolled in 'managePhantoms()')
            game.phantoms.erase(game.phantoms.begin() + n);
        }
    }

    // Update phantoms list
    manage_phantoms();
}


/*
    Check whether we need to increase the number of phantoms
    on the board or increase their speed -- all caused by a
    level-up. We up the level if all the level's phantoms have
    been zapped.
 */
void manage_phantoms() {
    bool level_up = false;

    // If we're on levels 1 and 2, we only have that number of
    // Phantoms. From 3 and up, there are aways three in the maze
    if (game.level < MAX_PHANTOMS) {
        if (game.level_kills == game.level) {
            game.level_kills = 0;
            ++game.level;
            ++game.phantom_count;
            level_up = true;
        }
    } else {
        if (game.level_kills == MAX_PHANTOMS) {
            game.level_kills = 0;
            level_up = true;
            ++game.level;
        }
    }

    // Did we level-up? Is so, update the phantom movement speed
    if (level_up) {
        uint8_t index = (game.level - 1) * 4;
        game.phantom_speed = ((PHANTOM_MOVE_TIME_US << level_data[index + 2]) >> level_data[index + 3]);
    }

    // Just in case...
    if (game.phantom_count > MAX_PHANTOMS) game.phantom_count = MAX_PHANTOMS;

    // Do we need to add any new phantoms to the board?
    if (game.phantoms.size() < game.phantom_count) {
        Phantom p = Phantom();
        p.roll_location();
        game.phantoms.push_back(p);
    }
}


/*
 *      GAME OUTCOMES
 */

/*
    The player has died -- show the map and the score.
 */
void death() {
    game.state = PLAYER_IS_DEAD;

    //for (unsigned int i = 400 ; i > 100 ; i -= 2) tone(i, 30, 0);
    sleep_ms(50);
    //tone(2200, 500, 600);

    clear();
    text("YOU", 0, 0);
    text("WERE", 0, 8);
    text("KILLED", 0, 16);

    text("PRESS", 0, 40);
    text("ANY", 0, 48);
    text("KEY", 0, 56);

    // Show the map
    //show_scores();
    //ssd1306_inverse(false);
}


/*
    Get player movement action from the joypad
    Favour movement over rotation
 */
uint8_t get_direction(uint8_t keys_pressed) {
    if (keys_pressed & 0x10) return MOVE_FORWARD;
    if (keys_pressed & 0x20) return MOVE_BACKWARD;
    if (keys_pressed & 0x40) return TURN_LEFT;
    if (keys_pressed & 0x80) return TURN_RIGHT;

    // Just in case
    return ERROR_CONDITION;
}

/*
    Return the index of the closest facing Phantom to the
    player from the the 'phantoms' array -- or ERROR_CONDITION.
    'range' is the number of squares we'll iterate over.
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


/*
    Return the number of Phantoms in front of the player
    'range' is the number of squares we'll iterate over.
 */
uint8_t count_facing_phantoms(uint8_t range) {
    uint8_t phantom_count = 0;
    switch(game.player.direction) {
        case DIRECTION_NORTH:
            if (game.player.y == 0) return phantom_count;
            if (game.player.y - range < 0) range = game.player.y;
            for (int8_t i = game.player.y ; i >= game.player.y - range ; --i) {
                phantom_count += (Map::phantom_on_square(game.player.x, i) != ERROR_CONDITION ? 1 : 0);;
            }
            break;
        case DIRECTION_EAST:
            if (game.player.x == 19) return phantom_count;
            if (game.player.x + range > 19) range = 19 - game.player.x;
            for (int8_t i = game.player.x ; i <= game.player.x + range ; ++i) {
                phantom_count += (Map::phantom_on_square(i, game.player.y) != ERROR_CONDITION ? 1 : 0);;
            }
            break;
        case DIRECTION_SOUTH:
            if (game.player.y == 19) return phantom_count;
            if (game.player.y + range > 19) range = 19 - game.player.y;
            for (int8_t i = game.player.y ; i <= game.player.y + range ; ++i) {
                phantom_count += (Map::phantom_on_square(game.player.x, i) != ERROR_CONDITION ? 1 : 0);;
            }
            break;
        default:
            if (game.player.x == 0) return phantom_count;
            if (game.player.x - range < 0) range = game.player.x;
            for (int8_t i = game.player.x ; i >= game.player.x - range ; --i) {
                phantom_count += (Map::phantom_on_square(i, game.player.y) != ERROR_CONDITION ? 1 : 0);;
            }
    }

    return phantom_count;
}