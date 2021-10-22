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

using namespace picosystem;


namespace Gfx {

/*
    Render a single viewpoint frame at the specified square.
    Progressively draw in walls, square by square, moving away
    from (x,y) in the specified direction.

    - Parameters:
        - x:          The square's X co-ordinate.
        - y:          The square's Y co-ordinate.
        - directions: The direction in which the viewer is facing.
 */
void draw_screen(uint8_t x, uint8_t y, uint8_t direction) {
    int8_t far_frame = Map::get_view_distance(x, y, direction);
    int8_t frame = far_frame;
    uint8_t phantom_count = count_facing_phantoms(far_frame);
    phantom_count = (phantom_count << 4) | phantom_count;
    uint8_t i = 0;

    pen(0, 0, 0);
    clear();
    pen(40, 40, 0);
    frect(0, 20, 240, 200);


    switch(direction) {
        case DIRECTION_NORTH:
            // Viewer is facing north, so left = West, right = East
            // Run through the squares from the view limit (inner frame) forward
            // to the player's current square (outer frame)
            i = y - far_frame;
            do {
                // Draw the current frame
                draw_section(x, i, DIRECTION_WEST, DIRECTION_EAST, frame, far_frame);

                // Check for the presence of a Phantom on the drawn square
                // and, if there is, draw it in
                // NOTE 'phantom_count comes back so we can keep track of multiple
                //      Phantoms in the player's field of view and space them
                //      laterally
                if (phantom_count > 0 && Map::phantom_on_square(x, i)) {
                    draw_phantom(frame, &phantom_count);
                }

                // Move to the next frame and square
                --frame;
                ++i;
            } while (frame >= 0);
        break;

        case DIRECTION_EAST:
            i = x + far_frame;
            do {
                draw_section(i, y, DIRECTION_NORTH, DIRECTION_SOUTH, frame, far_frame);
                if (phantom_count > 0 && Map::phantom_on_square(i, y)) {
                    draw_phantom(frame, &phantom_count);
                }
                --frame;
                --i;
            } while (frame >= 0);
            break;

        case DIRECTION_SOUTH:
            i = y + far_frame;
            do {
                draw_section(x, i, DIRECTION_EAST, DIRECTION_WEST, frame, far_frame);
                if (phantom_count > 0 && Map::phantom_on_square(x, i)) {
                    draw_phantom(frame, &phantom_count);
                }
                --frame;
                --i;
            } while (frame >= 0);
            break;

        default:
            i = x - far_frame;
            do {
                draw_section(i, y, DIRECTION_SOUTH, DIRECTION_NORTH, frame, far_frame);
                if (phantom_count > 0 && Map::phantom_on_square(i, y)) {
                    draw_phantom(frame, &phantom_count);
                }
                --frame;
                ++i;
            } while (frame >= 0);
    }
}

/*
    Draw a section of the view, ie. a frame.

    - Parameters:
        - x:              The square's X co-ordinate.
        - y:              The square's Y co-ordinate.
        - left_dir:       The direction to the left of the viewer.
        - rightt_dir:     The direction to the left of the viewer.
        - current_frame:  The viewpoint's frame index.
        - furthest_frame: The frame index of the last visble square.

    - Returns: `true` when we've got to the furthest rendered square,
               `false` otherwise
 */
bool draw_section(uint8_t x, uint8_t y, uint8_t left_dir, uint8_t right_dir, uint8_t current_frame, uint8_t furthest_frame) {
    // Is the square a teleporter? If so, draw it
    //if (x == game.tele_x && y == game.tele_y) draw_teleporter(current_frame);

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

    - Parameters:
        - frame_index: The frame index of the current frame.
 */
void draw_floor_line(uint8_t frame_index) {
    Rect r = rects[frame_index + 1];
    pen(40, 0, 0);
    // rect(r.x, r.y + 40, r.width, r.height);
    line(r.x, r.y + r.height + 40, r.x + r.width, r.y + r.height + 40);
}

/*
    Draw a green floor tile to indicate the Escape teleport location.
    When stepping on this, the player can beam to their start point.

    - Parameters:
        - frame_index: The frame index of the current frame.
 */
void draw_teleporter(uint8_t frame_index) {
    Rect r = rects[frame_index];
    pen(0, 40, 0);
    frect(r.x, r.y, r.width, r.height); // Needs modifying
}

/*
    Render a left-side wall section for the current square.
    NOTE 'is_open' is true if there is no wall -- ie. we're at
         a junction point.

    - Parameters:
        - frame_index: The frame index of the current frame.
        - is_open:     `true` if the wall is a path, `false` if it's a wall.
 */
void draw_left_wall(uint8_t frame_index, bool is_open) {
    // Get the 'i'ner and 'o'uter frames
    Rect i = rects[frame_index + 1];
    Rect o = rects[frame_index];

    // Draw an open left wall, ie. the facing wall of the
    // adjoining corridor, and then return
    pen(0, 0, 40);
    frect(o.x, i.y + 40, i.x - o.x - 1, i.height);
    if (is_open) return;

    // Add upper and lower triangles to present a wall section
    fpoly({o.x, o.y + 40, i.x - 2, i.y + 39, o.x, i.y + 39});
    fpoly({o.x, i.y + i.height + 39, i.x, i.y + i.height + 39, o.x, o.y + o.height + 40});
}


/*
    Render a right-side wall section for the current square.
    NOTE 'is_open' is true if there is no wall -- we're at
         a junction point.

    - Parameters:
        - frame_index: The frame index of the current frame.
        - is_open:     `true` if the wall is a path, `false` if it's a wall.
 */
void draw_right_wall(uint8_t frame_index, bool is_open) {
    // Get the 'i'ner and 'o'uter frames
    Rect i = rects[frame_index + 1];
    Rect o = rects[frame_index];

    // Draw an open left wall, ie. the facing wall of the
    // adjoining corridor, and then return
    pen(0, 0, 40);
    uint8_t xd = i.x + i.width;
    frect(xd + 1, i.y + 40, o.width + o.x - xd - 1, i.height);
    if (is_open) return;

    // Add upper and lower triangles to present a wall section
    fpoly({xd + 1, i.y + 39, o.x + o.width - 1, o.y + 40, o.x + o.width - 1, i.y + 39});
    fpoly({xd + 1, i.y + i.height + 39, o.x + o.width - 1, i.y + i.height + 39, o.x + o.width - 1, o.y + o.height + 40});
}

/*
    Draw the wall facing the viewer, or for very long distances,
    an 'infinity' view.

    - Parameters:
        - frame_index: The frame index of the current frame.
 */
void draw_far_wall(uint8_t frame_index) {
    Rect r = rects[frame_index + 1];
    pen(0, 0, 40);
    frect(r.x, r.y + 40, r.width, r.height);
}


/*
    Draw the laser sight: big cross on the screen.
 */
void draw_reticule() {
    pen(40, 0, 0);
    line(100, 120, 140, 120);
    line(120, 100, 120, 140);
}


void draw_zap() {

}

void animate_turn(bool is_right) {

}

void draw_phantom(uint8_t frame_number, uint8_t* phantom_count) {

}


}   // namespace Gfx
