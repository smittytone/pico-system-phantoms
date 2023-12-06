/*
 * Phantom Slayer
 * Phantom class code
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using std::string;


/*
 *      EXTERNALLY-DEFINED GLOBALS
 */
extern Game         game;
extern tinymt32_t   tinymt_store;


/*
    Constructor.

    NOTE This does not set the Phantom's location.

    - Parameters:
        - start_x: Inital X co-ordinate. Default: off the board.
        - start_y: Inital Y co-ordinate. Default: off the board.
 */
Phantom::Phantom() {
    // Use 'NOT_ON_BOARD' (== ERROR_CONDITION) as 'not on board yet'
    init();
    direction = DIRECTION_NORTH;
    x = NOT_ON_BOARD;
    y = NOT_ON_BOARD;
}


/**
    Roll the Phantom's hit points
 */
void Phantom::init() {
    uint8_t level_index = (game.level - 1) * 4;
    uint8_t min_hit_points = level_data[level_index];
    uint8_t max_hit_points = level_data[level_index + 1];
    hp = Utils::irandom(min_hit_points, max_hit_points);
    if (hp < 1) {
        hp = 1;
        printf("PH WITH 0 HP Min: %i, Max: %i\n", min_hit_points, max_hit_points);
    }
    back_steps = 0;
}


/*
    Set the Phantom on a new square.
 */
void Phantom::place(uint8_t my_index) {
    while (true) {
        // Pick a random co-ordinate
        uint8_t new_x = Utils::irandom(0, 20);
        uint8_t new_y = Utils::irandom(0, 20);

        // Make sure we're selecting a clear square, the player is not there
        // already and is not in a nearby square either
        if ((new_x < game.player.x - 4 || new_x > game.player.x + 4) && (new_y < game.player.y - 4 || new_y > game.player.y + 4)) {
            if (Map::get_square_contents(new_x, new_y) == MAP_TILE_CLEAR) {
                bool good = true;

                // Make sure the square is not occupied by another Phantom
                if (game.phantom_count > 1) {
                    for (uint8_t i = 0 ; i < game.phantom_count ; ++i) {
                        if (i == my_index) continue;
                        Phantom &p = game.phantoms.at(i);
                        if (p.x == new_x && p.y == new_y) {
                            good = false;
                            break;
                        }
                    }
                }

                if (good) {
                    x = new_x;
                    y = new_y;
                    break;
                }
            }
        }
    }
}


/*
    Move the Phantom.

    - Returns: `true` if the player was caught,
               otherwise `false`.
 */
bool Phantom::move() {
    // Has the Phantom been zapped? Don't move it
    if (x == NOT_ON_BOARD || hp < 1) return false;

    uint8_t new_x = x;
    uint8_t new_y = y;
    uint8_t new_direction = direction;
    uint8_t *p_x = &new_x;
    uint8_t *p_y = &new_y;

    // Get distance to player
    int8_t dx = x - game.player.x;
    int8_t dy = y - game.player.y;

    // Has the phantom got the player?
    if (dx == 0 && dy == 0) return true;

    // Set up direction storage
    uint8_t available_directions = 0;
    uint8_t favoured_directions = 0;
    uint8_t usable_directions = 0;
    uint8_t exit_count = 0;

    // Determine the directions in which the phantom *can* move: empty spaces with no phantom already there
    if (x > 0 && Map::get_square_contents(x - 1, y) != MAP_TILE_WALL && Map::phantom_on_square(x - 1, y) == ERROR_CONDITION) {
        available_directions |= PHANTOM_WEST;
        ++exit_count;
    }

    if (x < MAP_MAX && Map::get_square_contents(x + 1, y) != MAP_TILE_WALL && Map::phantom_on_square(x + 1, y) == ERROR_CONDITION) {
        available_directions |= PHANTOM_EAST;
        ++exit_count;
    }

    if (y > 0 && Map::get_square_contents(x, y - 1) != MAP_TILE_WALL && Map::phantom_on_square(x, y - 1) == ERROR_CONDITION) {
        available_directions |= PHANTOM_NORTH;
        ++exit_count;
    }

    if (y < MAP_MAX && Map::get_square_contents(x, y + 1) != MAP_TILE_WALL && Map::phantom_on_square(x, y + 1) == ERROR_CONDITION) {
        available_directions |= PHANTOM_SOUTH;
        ++exit_count;
    }

    if (available_directions == 0) {
        // Phantom can't move anywhere -- all its exits are currently blocked
        return false;
    }

    // FROM 1.0.1
    // Move away from the player if the Phantom is reversing
    uint8_t from = 0;
    if (back_steps > 0) {
        // Phantom is indeed reversing, so get the direction
        // from which it moved into this square -- we'll use this
        // to prevent the Phantom from back-tracking if in the next
        // lines it can go after the player again
        from = came_from();
        if (exit_count > 2) {
            // The Phantom has reached a junction, ie. a square with more than
            // two exits, so reset the reversal and try to move toward the player again
            back_steps = 0;
        } else {
            // The Phantom isn't at a junction, so calculate its
            // vector away from the player ('dx' and 'dy'
            // vector toward the player unless changed here)
            dx *= -1;
            dy *= -1;
        }
    }

    // Get the Phantom's move preferences -- the direction(s)
    // in which it would like to go
    if (dy > 0) favoured_directions |= PHANTOM_NORTH;
    if (dy < 0) favoured_directions |= PHANTOM_SOUTH;
    if (dx > 0) favoured_directions |= PHANTOM_WEST;
    if (dx < 0) favoured_directions |= PHANTOM_EAST;

    // FROM 1.0.1
    // Remove the way the Phantom has come from its list of
    // favoured directions. This is used when it's reversing
    // and has reached a junction (ie. 'from != 0')
    favoured_directions &= (~from);

    // Count up the number of ways favoured moves and available squares match
    uint8_t count = 0;
    for (uint8_t i = 0 ; i < 4 ; ++i) {
        if ((available_directions & (1 << i)) && (favoured_directions & (1 << i))) {
            // Phantom wants to go in a certain direction and it has an exit
            // so record this as a usable direction
            ++count;
            usable_directions |= (1 << i);
        }
    }

    // Handle the move itself
    if (count == 1) {
        // Only one way for the Phantom to go, so take it
        move_one_square(usable_directions, p_x, p_y);
        new_direction = usable_directions;
    } else if (count == 2) {
        // The Phantom has two ways to go, so pick one of them at random:
        // even roll go the first way (0); odd roll go the second (1)
        uint8_t r = (Utils::irandom(1, 100) % 2);
        uint8_t i = 0;
        while(true) {
            // Iterate through the directions until we have that can be
            // used and then is selectable (r == 0)
            if (usable_directions & (1 << i)) {
                if (r == 0) {
                    // Take this direction
                    move_one_square((usable_directions & (1 << i)), p_x, p_y);
                    new_direction = (usable_directions & (1 << i));
                    break;
                } else {
                    // Ignore this direction
                    r--;
                }
            }

            ++i;
            if (i > 3) i = 0;
        }
    } else {
        // Count == 0 -- this is the special case where phantom can't move
        //  where it wants so must move away or wait (if it has NOWHERE to go)
        if (available_directions != 0) {
            // Just pick a random available direction and take it,
            // but remove the way the phantom came (provided it
            // doesn't leave it with no way out)
            // NOTE re-calculate 'from' here so we don't mess up
            //      the ealier case when it needs to be zero
            from = came_from();
            uint8_t ad = available_directions;
            ad &= (~from);

            // Just in case removing from leaves the Phantom nowhere to go
            // ie. it's at a dead end
            if (ad != 0) available_directions = ad;

            // Pick a random value and count down through the available exits
            // until it comes to zero -- then take that one
            uint8_t i = 0;
            uint8_t r = Utils::irandom(0, 4);
            while (true) {
                if ((available_directions & (1 << i)) > 0) {
                    if (r == 0) {
                        move_one_square((available_directions & (1 << i)), p_x, p_y);
                        new_direction = (available_directions & (1 << i));
                        back_steps = 1;
                        break;
                    } else {
                        r--;
                    }
                }

                ++i;
                if (i > 3) i = 0;
            }
        }
    }

    // Set the Phantom's new location
    x = new_x;
    y = new_y;
    direction = new_direction;

    return false;
}


/*
    Move the Phantom one space according in the chosen direction.
 */
void Phantom::move_one_square(uint8_t nd, uint8_t *nx, uint8_t *ny) {
    if (nd == PHANTOM_NORTH) *ny = y - 1;
    if (nd == PHANTOM_SOUTH) *ny = y + 1;
    if (nd == PHANTOM_EAST)  *nx = x + 1;
    if (nd == PHANTOM_WEST)  *nx = x - 1;
}


/*
    Return the direction the phantom has come from.
 */
uint8_t Phantom::came_from() {
    if (direction == PHANTOM_WEST)  return PHANTOM_EAST;
    if (direction == PHANTOM_EAST)  return PHANTOM_WEST;
    if (direction == PHANTOM_NORTH) return PHANTOM_SOUTH;
    return PHANTOM_NORTH;
}
