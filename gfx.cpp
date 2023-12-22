/*
 * Phantom Slayer
 * Graphics Routines
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
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
extern uint8_t          dead_phantom_index;


/*
 *      GLOBALS
 */
const uint8_t           radii[5] = {20, 16, 12, 8, 4};
buffer_t*               word_buffer = buffer(82, 70, (void *)word_sprites);
buffer_t*               phantom_buffer = buffer(173, 150, (void *)phantom_sprites);
buffer_t*               zapped_buffer = buffer(173, 150, (void *)zapped_sprites);
buffer_t*               logo_buffer = buffer(212, 20, (void *)logo_sprite);
buffer_t*               credit_buffer = buffer(104, 34, (void *)credit_sprite);
color_t                 side_data[240 * 240] __attribute__ ((aligned (4))) = {0};
buffer_t*               side_buffer = buffer(240, 240, side_data);


namespace Gfx {


/**
 * @brief Render a single viewpoint frame at the specified square.
 *        Progressively draw in walls, square by square, moving away
 *        from (x,y) in the specified direction.
 *
 * @param x:          The square's X co-ordinate.
 * @param y:          The square's Y co-ordinate.
 * @param directions: The direction in which the viewer is facing.
 */
void draw_screen(uint8_t x, uint8_t y, uint8_t direction) {

    uint8_t far_frame = Map::get_view_distance(x, y, direction);
    int8_t frame = far_frame;
    int8_t phantom_frame = frame;

    // Set 'phantom_count' upper nibble to total number of
    // Phantoms facing the player; lower nibble is the 'current'
    // Phantom if the player can see more than one
    uint8_t phantom_count = count_facing_phantoms(far_frame);
    phantom_count = (phantom_count << 4) | phantom_count;
    uint8_t i = 0;

    // Clear the screen
    cls(COLOURS::BLACK);

#ifdef DEBUG
    pen((color_t)COLOURS::WHITE);
    draw_number(phantom_count & 0x0F, 200, 230);
#endif

    if (game.state == GAME_STATE::DO_TELEPORT_ONE || game.state == GAME_STATE::PHANTOM_DEAD) {
        // Background red
        pen((color_t)COLOURS::RED);
    } else if (game.state == GAME_STATE::ZAP_PHANTOM) {
        // Background blue
        pen((color_t)COLOURS::BLUE);
    } else {
        // Background yellow
        pen((color_t)COLOURS::YELLOW);
    }

    // File the drawing area
    frect(0, 40, 240, 160);

    bool phantom_is_hit = (game.state == GAME_STATE::PHANTOM_DEAD || game.state == GAME_STATE::ZAP_PHANTOM);
    switch((DIRECTION)direction) {
        case DIRECTION::NORTH:
            // Viewer is facing north, so left = West, right = East
            // Run through the squares from the view limit (inner frame) forward
            // to the player's current square (outer frame)
            i = y - far_frame;
            do {
                // Draw the current frame
                draw_section(x, i, (uint8_t)DIRECTION::WEST, (uint8_t)DIRECTION::EAST, frame, far_frame);

                // Move to the next frame and square
                --frame;
                ++i;
            } while (frame >= 0);

            draw_skirting(far_frame);
            i = y - far_frame;

            do {
                // Check for the presence of a Phantom on the drawn square
                // and, if there is, draw it in
                // NOTE 'phantom_count comes back so we can keep track of multiple
                //      Phantoms in the player's field of view and space them
                //      laterally
                uint8_t n = Map::phantom_on_square(x, i);
                if (phantom_count > 0 && n != NONE) {
                    draw_phantom(phantom_frame, &phantom_count, phantom_is_hit);
                }

                // Move to the next frame and square
                --phantom_frame;
                ++i;
            } while (phantom_frame >= 0);
        break;

        case DIRECTION::EAST:
            i = x + far_frame;
            do {
                draw_section(i, y, (uint8_t)DIRECTION::NORTH, (uint8_t)DIRECTION::SOUTH, frame, far_frame);
                --frame;
                --i;
            } while (frame >= 0);

            draw_skirting(far_frame);
            i = x + far_frame;

            do {
                uint8_t n = Map::phantom_on_square(i, y);
                if (phantom_count > 0 && n != NONE) {
                    draw_phantom(phantom_frame, &phantom_count, phantom_is_hit);
                }
                --phantom_frame;
                --i;
            } while (phantom_frame >= 0);

            break;

        case DIRECTION::SOUTH:
            i = y + far_frame;
            do {
                draw_section(x, i, (uint8_t)DIRECTION::EAST, (uint8_t)DIRECTION::WEST, frame, far_frame);
                --frame;
                --i;
            } while (frame >= 0);

            draw_skirting(far_frame);
            i = y + far_frame;

            do {
                uint8_t n = Map::phantom_on_square(x, i);
                if (phantom_count > 0 && n != NONE) {
                    draw_phantom(phantom_frame, &phantom_count, phantom_is_hit);
                }
                --phantom_frame;
                --i;
            } while (phantom_frame >= 0);

            break;

        default:
            i = x - far_frame;
            do {
                draw_section(i, y, (uint8_t)DIRECTION::SOUTH, (uint8_t)DIRECTION::NORTH, frame, far_frame);
                --frame;
                ++i;
            } while (frame >= 0);

            draw_skirting(far_frame);
            i = x - far_frame;

            do {
                uint8_t n = Map::phantom_on_square(i, y);
                if (phantom_count > 0 && n != NONE) {
                    draw_phantom(phantom_frame, &phantom_count, phantom_is_hit);
                }
                --phantom_frame;
                ++i;
            } while (phantom_frame >= 0);
    }

    // FROM 1.2.0
    // Draw over protruding diagonals below the view
    pen((color_t)COLOURS::BLACK);
    hline(0, 200, 240);
}


/**
 * @brief Draw a section of the view, ie. a frame.
 *
 * @param x:              The square's X co-ordinate.
 * @param y:              The square's Y co-ordinate.
 * @param left_dir:       The direction to the left of the viewer.
 * @param right_dir:     The direction to the left of the viewer.
 * @param current_frame:  The viewpoint's frame index.
 * @param furthest_frame: The frame index of the last visble square.
 * @returns: `true` when we've got to the furthest rendered square,
 *             `false` otherwise
 */
bool draw_section(uint8_t x, uint8_t y, uint8_t left_dir, uint8_t right_dir, uint8_t current_frame, uint8_t furthest_frame) {

    // Is the square a teleporter? If so, draw it
    if (x == game.tele_x && y == game.tele_y) draw_teleporter(current_frame);

    // Draw in left and right wall segments
    // NOTE Second argument is true or false: wall section is
    //      open or closed, respectively
    if (game.state != GAME_STATE::PHANTOM_DEAD) {
        draw_left_wall(current_frame, (Map::get_view_distance(x, y, left_dir) > 0));
        draw_right_wall(current_frame, (Map::get_view_distance(x, y, right_dir) > 0));

        // Have we reached the furthest square the viewer can see?
        if (current_frame == furthest_frame) {
            draw_far_wall(current_frame);
            return true;
        }
    }

    // Draw a line on the floor
    draw_floor_line(current_frame);
    return false;
}


/**
 * @brief Draw a grid line on the floor -- this is all
 *        we do to create the floor (ceiling has no line)
 *
 * @param frame_index: The frame index of the current frame.
 */
void draw_floor_line(uint8_t frame_index) {

    Rect r = rects[frame_index + 1];
    auto colour = (color_t)COLOURS::RED;
    if (game.state == GAME_STATE::DO_TELEPORT_ONE) colour = (color_t)COLOURS::WHITE;
    if (game.state == GAME_STATE::PHANTOM_DEAD || game.state == GAME_STATE::ZAP_PHANTOM) colour = (color_t)COLOURS::GREEN;
    pen(colour);
    hline(r.x, r.y + r.height + 39, r.width);
    hline(r.x - 1 , r.y + r.height + 40, r.width + 2);
}


/**
 * @brief Draw diagonal lines where floor meets wall.
 *
 * @param frame_index: The frame index of the current frame.
 */
void draw_skirting(uint8_t frame_index) {

    Rect r = rects[frame_index + 1];
    auto colour = (color_t)COLOURS::RED;
    if (game.state == GAME_STATE::DO_TELEPORT_ONE) colour = (color_t)COLOURS::WHITE;
    if (game.state == GAME_STATE::PHANTOM_DEAD || game.state == GAME_STATE::ZAP_PHANTOM) colour = (color_t)COLOURS::GREEN;
    pen(colour);
    line(r.x, r.y + r.height + 39, 0, 199);
    line(r.x, r.y + r.height + 40 , 0, 200);
    line(r.x + r.width, r.y + r.height + 39, 240, 199);
    line(r.x + r.width, r.y + r.height + 40, 240, 200);
    hline(r.x, r.y + r.height + 39, r.width);
    hline(r.x - 1 , r.y + r.height + 40, r.width + 2);
}


/**
 * @brief Draw a green floor tile to indicate the Escape teleport location.
 *        When stepping on this, the player can beam to their start point.
 *
 * @param frame_index: The frame index of the current frame.
 */
void draw_teleporter(uint8_t frame_index) {

    Rect c = rects[frame_index];
    Rect b = rects[frame_index + 1];
    pen((color_t)COLOURS::GREEN);
    frect(c.x, b.y + b.height + 40, c.width, (c.y + c.height) - (b.y + b.height));
}


/**
 * @brief Render a left-side wall section for the current square.
 *        NOTE `is_open` is `true` if there is no wall -- ie. we're at
 *             a junction point.
 *
 * @param frame_index: The frame index of the current frame.
 * @param is_open:     `true` if the wall is a path, `false` if it's a wall.
 */
void draw_left_wall(uint8_t frame_index, bool is_open) {

    // Get the 'i'ner and 'o'uter frames
    Rect i = rects[frame_index + 1];
    Rect o = rects[frame_index];

    // Draw an open left wall, ie. the facing wall of the
    // adjoining corridor, and then return
    auto colour = (color_t)COLOURS::BLUE;
    if (game.state == GAME_STATE::DO_TELEPORT_ONE) colour = (color_t)COLOURS::WHITE;
    if (game.state == GAME_STATE::ZAP_PHANTOM) colour = (color_t)COLOURS::YELLOW;
    pen(colour);
    frect(o.x, i.y + 40, i.x - o.x - 1, i.height);
    if (is_open) return;

    // Add upper and lower triangles to present a wall section
    fpoly({o.x, o.y + 40, i.x - 2, i.y + 39, o.x, i.y + 39});
    fpoly({o.x, i.y + i.height + 39, i.x, i.y + i.height + 39, o.x, o.y + o.height + 40});
}


/**
 * @brief Render a right-side wall section for the current square.
 *        NOTE `is_open` is `true` if there is no wall -- we're at
 *             a junction point.
 *
 * @param frame_index: The frame index of the current frame.
 * @param is_open:     `true` if the wall is a path, `false` if it's a wall.
 */
void draw_right_wall(uint8_t frame_index, bool is_open) {

    // Get the 'i'ner and 'o'uter frames
    Rect i = rects[frame_index + 1];
    Rect o = rects[frame_index];

    // Draw an open left wall, ie. the facing wall of the
    // adjoining corridor, and then return
    auto colour = (color_t)COLOURS::BLUE;
    if (game.state == GAME_STATE::DO_TELEPORT_ONE) colour = (color_t)COLOURS::WHITE;
    if (game.state == GAME_STATE::ZAP_PHANTOM) colour = (color_t)COLOURS::YELLOW;
    pen(colour);
    uint8_t xd = i.x + i.width;
    frect(xd + 1, i.y + 40, o.width + o.x - xd - 1, i.height);
    if (is_open) return;

    // Add upper and lower triangles to present a wall section
    fpoly({xd + 1, i.y + 39, o.x + o.width - 1, o.y + 40, o.x + o.width - 1, i.y + 39});
    fpoly({xd + 1, i.y + i.height + 39, o.x + o.width - 1, i.y + i.height + 39, o.x + o.width - 1, o.y + o.height + 40});
}


/**
 * @brief Draw the wall facing the viewer, or for very long distances,
 *        an 'infinity' view.
 *
 * @param frame_index: The frame index of the current frame.
 */
void draw_far_wall(uint8_t frame_index) {

    Rect r = rects[frame_index + 1];

    if (frame_index == MAX_VIEW_RANGE) {
        uint8_t ryd = r.y + r.height;
        uint8_t rxd = r.x + r.width;
        fpoly({r.x,     r.y + 39,
               r.x + 4, r.y + 42,
               r.x + 4, ryd + 37,
               r.x,     ryd + 39});
        fpoly({rxd,     r.y + 39,
               rxd,     ryd + 39,
               rxd - 4, ryd + 37,
               rxd - 4, r.y + 42});
    } else {
        auto colour = (color_t)COLOURS::BLUE;
        if (game.state == GAME_STATE::DO_TELEPORT_ONE) colour = (color_t)COLOURS::WHITE;
        if (game.state == GAME_STATE::ZAP_PHANTOM) colour = (color_t)COLOURS::YELLOW;
        pen(colour);
        frect(r.x, r.y + 40, r.width, r.height);
    }
}


/**
 * @brief Draw the laser sight: a big cross on the screen.
 */
void draw_reticule(void) {

    pen((color_t)COLOURS::ORANGE);
    rect(100 + game.crosshair_delta, 119, 40, 2);
    rect(119 + game.crosshair_delta, 100, 2, 40);
}


/**
 * @brief Draw a laser bolt.
 *
 * @param frame_index: The frame in which to place the bolt.
 */
void draw_zap(uint8_t frame_index) {

    if (frame_index < MAX_VIEW_RANGE) {
        uint16_t radius = radii[frame_index];
        pen((color_t)COLOURS::ORANGE);
        fcircle(120, 120, radius);
    }
}


/**
 * @brief Draw a Phantom in the specified frame - which determines
 *        its x and y co-ordinates in the frame.
 *
 * @param frame_index: The frame in which to place the Phantom.
 * @param count:       The number of Phantoms on screen.
 */
void draw_phantom(uint8_t frame_index, uint8_t* count, bool is_zapped) {

    Rect r = rects[frame_index];
    uint8_t dx = 120;
    uint8_t c = *count;
    uint8_t number_phantoms = (c >> 4);
    uint8_t current = c & 0x0F;
    game.crosshair_delta = 0;

    // Space the phantoms sideways according to
    // the number of them on screen
    if (number_phantoms > 1) {
        if (current == 2) {
            dx = 120 - r.spot;
            game.crosshair_delta = 0 - r.spot;
        }

        if (current == 1) {
            dx = 120 + r.spot;
            game.crosshair_delta = r.spot;
        }

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
    if (is_zapped) pen((color_t)COLOURS::WHITE);
    blit(is_zapped ? zapped_buffer : phantom_buffer, sx, sy, width, height, dx, dy);
}


/**
 * @brief Display a pre-rendered word graphic.
 *
 * @param index: The word's index in the `word_sizes` array.
 * @param x:     The target X co-ordinate.
 * @param y:     The target Y co-ordinate.
 */
void draw_word(WORDS index, uint8_t x, uint8_t y, bool do_double) {

    uint8_t w_x = word_sizes[(uint8_t)index * 3];
    uint8_t w_y = word_sizes[(uint8_t)index * 3 + 1];
    uint8_t w_len = word_sizes[(uint8_t)index * 3 + 2];


    if (do_double) {
        blit(word_buffer, w_x, w_y, w_len, 10, x, y, (w_len << 1), 20);
    } else {
        blit(word_buffer, w_x, w_y, w_len, 10, x, y);
    }
}


/**
 * @brief Display a pre-rendered single-digit number graphic.
 *
 * @param index:     The word's index in the `word_sizes` array.
 * @param x:         The target X co-ordinate.
 * @param y:         The target Y co-ordinate.
 * @param do_double: Render at 2x size.
 */
void draw_number(uint8_t number, uint8_t x, uint8_t y, bool do_double) {

    uint8_t n_len = number == 1 ? 2 : 6;
    uint8_t n_x = number == 1 ? 6 : (2 + ((number - 1) * 6));
    if (number == 0) n_x = 0;

    if (do_double) {
        blit(word_buffer, n_x, 0, n_len, 10, x, y, (n_len << 1), 20);
    } else {
        blit(word_buffer, n_x, 0, n_len, 10, x, y);
    }
}


/**
 * @brief Roll up the logo down from the top of the screen.
 *
 * @param y: Y coordinate at which to draw the logo.
 */
void animate_logo(int16_t y) {

    if (y < -19) return;

    uint8_t height = 21 + y;
    if (height > 22) height = 22;
    uint8_t top = 0;
    if (y > 0) top = y - 1;
    frect(14, top, 212, height);

    uint8_t delta = 20 + y;
    if (delta > 20) delta = 20;
    if (y < 0) y = 0;
    blit(logo_buffer, 0, 20 - delta, 212, delta, 14, y);
}


/**
 * @brief Roll up the credit from the bottom.
 *
 * @param y: Y coordinate at which to draw the credit.
 */
void animate_credit(int16_t y) {

    if (y > 239) return;

    uint8_t height = 241 - y;
    if (height > 36) height = 36;
    frect(68, y - 1, 104, height);

    height = y - 240;
    if (height > 34) height = 34;
    blit(credit_buffer, 0, 0, 104, height, 68, y);
}


/**
 * @brief Draw the side view - the view the player will see next -
 *        to the side buffer.
 */
void animate_turn() {

    // Draw the side view
    target(side_buffer);
    cls(COLOURS::BLACK);
    draw_screen(game.player.x, game.player.y, (uint8_t)game.player.direction);

    // Reset back to the main display
    target(SCREEN);
    blend(COPY);
}


/**
 * @brief Streamlined (sort of) blit code for left and right turn animations.
 *
 * @param src: Pointer to the source buffer.
 * @param sx:  The top-left corner X co-ordinate of the area to copy.
 * @param sy:  The top-left corner Y co-ordinate of the area to copy.
 * @param w:   The width of the area to copy.
 * @param h:   The height of the area to copy.
 * @param dx:  The top-left corner X co-ordinate of the area to copy to.
 * @param dy:  The top-left corner Y co-ordinate of the area to copy to.
 */
void alt_blit(const buffer_t *src, int32_t sx, int32_t sy, int32_t w, int32_t h, int32_t dx, int32_t dy) {

    color_t *ps = src->data + (sx + sy * src->w);
    color_t *pd = _dt->data + (dx + dy * _dt->w);
    int32_t ds = _dt->w;

    while (h--) {
        memcpy(pd, ps, w * 2);
        pd += ds;
        ps += src->w;
    }
}


/**
 * @brief Clear the screen to the required colour.
 *
 * @param colour: The chosen colour value.
*/
void cls(COLOURS colour) {

    pen((color_t)colour);
    clear();
}


}   // namespace Gfx
