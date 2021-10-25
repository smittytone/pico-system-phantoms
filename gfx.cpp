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
using std::string;


/*
 *      EXTERNALLY-DEFINED GLOBALS
 */
extern tinymt32_t       tinymt_store;
extern Game             game;
extern Rect             rects[7];


/*
 *      GLOBALS
 */
const uint8_t           radii[5] = {20, 16, 12, 8, 4};
buffer_t*               word_buffer = buffer(68, 50, (void *)word_sprites);
buffer_t*               phantom_buffer = buffer(173, 150, (void *)phantom_sprites);


namespace Gfx {

/**
    Render a single viewpoint frame at the specified square.
    Progressively draw in walls, square by square, moving away
    from (x,y) in the specified direction.

    - Parameters:
        - x:          The square's X co-ordinate.
        - y:          The square's Y co-ordinate.
        - directions: The direction in which the viewer is facing.
 */
void draw_screen(uint8_t x, uint8_t y, uint8_t direction) {
    uint8_t far_frame = Map::get_view_distance(x, y, direction);
    int8_t frame = far_frame;

    // Set 'phantom_count' upper nibble to total number of
    // Phantoms facing the player; lower nibble is the 'current'
    // Phantom if the player can see more than one
    uint8_t phantom_count = count_facing_phantoms(far_frame);
    phantom_count = (phantom_count << 4) | phantom_count;
    uint8_t i = 0;

    // Set the background
    pen(0, 0, 40);
    frect(0, 0, 240, 200);
    pen(40, 36, 0);
    frect(0, 40, 240, 160);

    if (game.player.x < 10) {
        draw_number(game.player.x, 0, 0);
    } else {
        draw_number(1, 0, 0);
        draw_number(x - 10, 4, 0);
    }

    if (game.player.y < 10) {
        draw_number(game.player.y, 0, 12);
    } else {
        draw_number(1, 0, 12);
        draw_number(game.player.y - 10, 4, 12);
    }

    draw_number(game.player.direction, 0, 24);

    draw_number((phantom_count >> 4), 160, 0);

    if (far_frame < 10) {
        draw_number(far_frame, 80, 0);
    } else {
        draw_number(1, 80, 0);
        draw_number(far_frame - 10, 84, 0);
    }

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
                if (phantom_count > 0 && Map::phantom_on_square(x, i) != ERROR_CONDITION) {
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
                if (phantom_count > 0 && Map::phantom_on_square(i, y) != ERROR_CONDITION) {
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
                if (phantom_count > 0 && Map::phantom_on_square(x, i) != ERROR_CONDITION) {
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
                if (phantom_count > 0 && Map::phantom_on_square(i, y) != ERROR_CONDITION) {
                    draw_phantom(frame, &phantom_count);
                }
                --frame;
                ++i;
            } while (frame >= 0);
    }
}

/**
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

/**
    Draw a grid line on the floor -- this is all
    we do to create the floor (ceiling has no line)

    - Parameters:
        - frame_index: The frame index of the current frame.
 */
void draw_floor_line(uint8_t frame_index) {
    Rect r = rects[frame_index + 1];
    pen(40, 0, 0);
    line(r.x, r.y + r.height + 39, r.x + r.width, r.y + r.height + 39);
    line(r.x -1 , r.y + r.height + 40, r.x + r.width + 1, r.y + r.height + 40);
}


/**
    Draw a green floor tile to indicate the Escape teleport location.
    When stepping on this, the player can beam to their start point.

    - Parameters:
        - frame_index: The frame index of the current frame.
 */
void draw_teleporter(uint8_t frame_index) {
    Rect c = rects[frame_index];
    Rect b = rects[frame_index + 1];
    pen(0, 40, 0);
    frect(c.x, b.y + b.height + 40, c.width, (c.y + c.height) - (b.y + b.height));

    /*
    bool dot_state = true;

    // Plot a dot pattern
    for (uint8_t y = r.y + r.height - 4; y < r.y + r.height ; ++y) {
        for (uint8_t i = r.x ; i < r.x + r.width - 2; i += 2) {
            pixel(dot_state ? i : i + 1, y);
        }
        dot_state = !dot_state;
    }
    */
}


/**
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


/**
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

/**
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


/**
    Draw the laser sight: big cross on the screen.
 */
void draw_reticule() {
    pen(0, 40, 0);
    rect(100, 119, 40, 2);
    rect(119, 100, 2, 40);
}


void draw_zap(uint8_t frame) {
    if (frame < 5) {
        uint16_t radius = radii[frame];
        pen(40, 40, 40);
        fcircle(120, 120, radius);
    }
}


void animate_turn(bool is_right) {

}


void draw_phantom(uint8_t frame_index, uint8_t* count) {
    // Draw a Phantom in the specified frame - which determines
    // its x and y co-ordinates in the frame
    Rect r = rects[frame_index];
    uint8_t dx = 120;
    uint8_t c = *count;
    uint8_t number_phantoms = (c >> 4);
    uint8_t current = c & 0x0F;

    // Space the phantoms sideways ccording to
    // the number of them on screen
    if (number_phantoms > 1) {
        if (current == 2) dx = 120 - r.spot;
        if (current == 1) dx = 120 + r.spot;
        *count = c - 1;
    }

    // NOTE Screen render frame indices run from 0 to 5, front to back
    uint8_t height = phantom_sizes[frame_index * 2];
    uint8_t width =  phantom_sizes[frame_index * 2 + 1];
    uint8_t sx =  0;
    uint8_t sy =  0;
    uint8_t dy = 120 - (height >> 1);
    dx -= (width >> 1);

    if (frame_index > 0) {
        for (uint8_t i = 0 ; i < frame_index ; i++) {
            sx += phantom_sizes[i * 2 + 1];
        }
    }

    // Paint in the Phantom
    blit(phantom_buffer, sx, sy, width, height, dx, dy);
}


void draw_text(int8_t x, int8_t y, string the_string, bool do_wrap) {
    // Print the supplied string at (x,y) (top-left co-ordinate), wrapping to the next line
    // if required. 'do_double' selects double-height output (currently not working)
    uint8_t space_size = 4;
    uint8_t bit_max = 16;
    pen(40, 0, 40);

    return;
    for (size_t i = 0 ; i < the_string.size() ; ++i) {
        uint8_t glyph[6];
        uint8_t col_1 = 0;
        uint8_t col_0 = 0;
        ssize_t glyph_len = 0;

        uint8_t asc_val = the_string[i] - 32;
        glyph_len = CHARSET[asc_val][0] + 1;
        for (size_t j = 0 ; j < glyph_len ; ++j) {
            if (j == glyph_len - 1) {
                glyph[j] = 0x00;
            } else {
                glyph[j] = CHARSET[asc_val][j + 1];
            }
        }

        col_0 = glyph[0];
        printf("Glyph LEN: %i",glyph_len);

        if (do_wrap) {
            if ((x + glyph_len * 2 >= 240)) {
                if (y + bit_max < 240) {
                    x = 0;
                    y += bit_max;
                } else {
                    return;
                }
            }
        }

        /*
        for (size_t j = 1 ; j < glyph_len + 1 ; ++j) {
            if (j == glyph_len) {
                //if (do_double) break;
                col_1 = glyph[j - 1];
            } else {
                col_1 = glyph[j];
            }

            uint16_t col_0_right = 0;
            uint16_t col_1_right = 0;
            uint16_t col_0_left = 0;
            uint16_t col_1_left = 0;

            col_0_right = text_stretch(col_0);
            col_0_left = col_0_right;
            col_1_right = text_stretch(col_1);
            col_1_left = col_1_right;

            for (int8_t a = 6 ; a >= 0 ; --a) {
                for (uint8_t b = 1 ; b < 3 ; b++) {
                    if ((((col_0 >> a) & 3) == 3 - b) && (((col_1 >> a) & 3) == b)) {
                        col_0_right |= (1 << ((a * 2) + b));
                        col_1_left |= (1 << ((a * 2) + 3 - b));
                    }
                }
            }

            for (uint8_t k = 0 ; k < bit_max ; ++k) {
                if (x < 240 && col_0_left & (1 << k)) pixel(x, y + k);
                if (x + 1 < 240 && col_0_right & (1 << k)) pixel(x + 1, y + k);
                if (x + 2 < 240 && col_1_left & (1 << k)) pixel(x + 2, y + k);
                if (x + 3 < 240&& col_1_right & (1 << k)) pixel(x + 3, y + k);
            }

            x += 2;

            if (x >= 240) {
                if (!do_wrap) return;
                if (y + bit_max < 240) {
                    x = 0;
                    y += bit_max;
                } else {
                    break;
                }
            }

            col_0 = col_1;
        }
        */
    }
}

/**
    Pixel-doubles an 8-bit value to 16 bits.
 */
uint16_t text_stretch(uint8_t x) {
    uint16_t v = (x & 0xF0) << 4 | (x & 0x0F);
    v = (v << 2 | v) & 0x3333;
    v = (v << 1 | v) & 0x5555;
    v |= (v << 1);
    return v;
}


void draw_word(uint8_t index, uint8_t x, uint8_t y) {
    uint8_t w_x = word_sizes[index * 3];
    uint8_t w_y = word_sizes[index * 3 + 1];
    uint8_t w_len = word_sizes[index * 3 + 2];
    blit(word_buffer, w_x, w_y, w_len, 10, x, y);
}


void draw_number(uint8_t number, uint8_t x, uint8_t y, bool do_double) {
    uint8_t w_len = number == 1 ? 2 : 6;
    uint8_t w_x = number == 1 ? 6 : (2 + (number - 1) * 6);
    if (number == 0) w_x = 0;
    if (do_double) {
        blit(word_buffer, w_x, 40, w_len, 10, x, y, (w_len << 1), 20);
    } else {
        blit(word_buffer, w_x, 40, w_len, 10, x, y);
    }
}


}   // namespace Gfx
