/*
 * Phantom Slayer
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using namespace Picosystem;

namespace Gfx {

/*
    Render a single viewpoint frame at the specified square.
    Progressively draw in walls, square by square, moving away
    from (x,y) in the specified direction.
 */
void draw_screen(uint8_t x, uint8_t y, uint8_t direction) {
    int8_t last_frame = Map::get_view_distance(x, y, direction);
    int8_t frame = last_frame;
    uint8_t phantom_count = count_facing_phantoms(last_frame);
    phantom_count = (phantom_count << 4) | phantom_count;
    uint8_t i = 0;

    switch(direction) {
        case DIRECTION_NORTH:
            // Viewer is facing north, so left = West, right = East
            // Run through the squares from the view limit (inner frame) forward
            // to the player's current square (outer frame)
            i = y - last_frame;
            do {
                // Draw the current frame
                draw_section(x, i, DIRECTION_WEST, DIRECTION_EAST, frame, last_frame);

                // Check for the presence of a Phantom on the drawn square
                // and, if there is, draw it in
                // NOTE 'phantom_count comes back so we can keep track of multiple
                //      Phantoms in the player's field of view and space them
                //      laterally
                if (phantom_count > 0 && locate_phantom(x, i) != ERROR_CONDITION) {
                    draw_phantom(frame, &phantom_count);
                }

                // Move to the next frame and square
                --frame;
                ++i;
            } while (frame >= 0);
        break;

        case DIRECTION_EAST:
            i = x + last_frame;
            do {
                draw_section(i, y, DIRECTION_NORTH, DIRECTION_SOUTH, frame, last_frame);
                if (phantom_count > 0 && locate_phantom(i, y) != ERROR_CONDITION) {
                    draw_phantom(frame, &phantom_count);
                }
                --frame;
                --i;
            } while (frame >= 0);
            break;

        case DIRECTION_SOUTH:
            i = y + last_frame;
            do {
                draw_section(x, i, DIRECTION_EAST, DIRECTION_WEST, frame, last_frame);
                if (phantom_count > 0 && locate_phantom(x, i) != ERROR_CONDITION) {
                    draw_phantom(frame, &phantom_count);
                }
                --frame;
                --i;
            } while (frame >= 0);
            break;

        default:
            i = x - last_frame;
            do {
                draw_section(i, y, DIRECTION_SOUTH, DIRECTION_NORTH, frame, last_frame);
                if (phantom_count > 0 && locate_phantom(i, y) != ERROR_CONDITION) {
                    draw_phantom(frame, &phantom_count);
                }
                --frame;
                ++i;
            } while (frame >= 0);
    }
}

/*
    Draw a section of the view, ie. a frame.

    - Returns: `true` when we've got to the furthest rendered square,
               `false` otherwise
 */
bool draw_section(uint8_t x, uint8_t y, uint8_t left_dir, uint8_t right_dir, uint8_t current_frame, uint8_t furthest_frame) {
    // Is the square a teleporter? If so, draw it
    if (x == game.tele_x && y == game.tele_y) draw_teleporter(current_frame);

    // Draw in left and right wall segments
    // NOTE Second argument is true or false: wall section is
    //      open or closed, respectively
    draw_left_wall(current_frame, (Map::get_view_distance(x, y, left_dir) > 0));
    draw_right_wall(current_frame, (Map::get_view_distance(x, y, right_dir) > 0));

    // Have we reached the furthest square the viewer can see?
    if (current_frame == furthest_frame) {
        draw_far_wall(current_frame);
        return true;
    }

    // Draw a line on the floor
    draw_floor_line(current_frame);
    return false;
}

/*
    Draw a grid line on the floor -- this is all
    we do to create the floor (ceiling has no line)
 */
void draw_floor_line(uint8_t frame_index) {
    Rect r = rects[frame_index + 1];
    pen(1,0,0);
    line(r.x - 1, r.y + r.height, r.x + r.width + 1, r.y + r.height);
}

/*
    Draw a green floor tile to indicate the Escape teleport location.
    When stepping on this, the player can beam to their start point
 */
void draw_teleporter(uint8_t frame_index) {
    Rect r = rects[frame_index];
    pen(0,1,0);
    frect(r.x, r.y, r.width, r.height); // Needs modifying
}

/*
    Render a left-side wall section for the current square.
    NOTE 'is_open' is true if there is no wall -- ie. we're at
         a junction point
 */
void draw_left_wall(uint8_t frame_index, bool is_open) {
    // Get the 'i'ner and 'o'uter frames
    Rect i = rects[frame_index + 1];
    Rect o = rects[frame_index];

    // Draw an open left wall, ie. the facing wall of the
    // adjoining corridor, and then return
    pen(0,0,1);
    frect(o.x, i.y, i.x - o.x - 1, i.height);
    if (is_open) return;

    // Add upper and lower triangles to present a wall section
    fpoly({o.x, o.y, i.x, i.y, o.x, i.y});
    fpoly({o.x, i.y, i.x, i.y, o.x, o.y});
}

/*
    Render a right-side wall section for the current square.
    NOTE 'is_open' is true if there is no wall -- we're at
         a junction point
 */
void draw_right_wall(uint8_t frame_index, bool is_open) {
    // Get the 'i'ner and 'o'uter frames
    Rect i = rects[frame_index + 1];
    Rect o = rects[frame_index];

    // Draw an open left wall, ie. the facing wall of the
    // adjoining corridor, and then return
    pen(0,0,1);
    uint8_t xd = i.width + i.x;
    frect(xd + 1, i.y, (o.width + o.x) - xd - 1, i.height);
    if (is_open) return;

    // Add upper and lower triangles to present a wall section
    fpoly({o.x + o.width, i.y, o.x + o.width, o.y, o.x + o.width, i.y});
    fpoly({o.x + o.width, i.y + i.height, o.x + o.width, o.y + o.height, o.x + o.width, i.y + i.height});
}

/*
    Draw the wall facing the viewer, or for very long distances,
    an 'infinity' view
 */
void draw_far_wall(uint8_t frame_index) {
    Rect r = rects[frame_index + 1];
    pen(0,0,1);
    frect(r.x, r.y, r.width, r.height);
}


/*
    Draw the laser sight: big cross on the screen.
 */
void Gfx::draw_reticule() {
    pen(1,1,0);
    line(100, 120, 140, 120);
    line(120, 100, 120, 140);
}


}   // namespace Gfx
