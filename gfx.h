/*
 * Phantom Slayer
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _GFX_UTILS_HEADER_
#define _GFX_UTILS_HEADER_

/*
 *      PROTOTYPES
 */
namespace Gfx {
    void        draw_screen(uint8_t x, uint8_t y, uint8_t direction);
    bool        draw_section(uint8_t x, uint8_t y, uint8_t left_dir, uint8_t right_dir, uint8_t current_frame, uint8_t furthest_frame);
    void        draw_floor_line(uint8_t frame_index);
    void        draw_teleporter(uint8_t frame_index);
    void        draw_left_wall(uint8_t frame_index, bool is_open);
    void        draw_right_wall(uint8_t frame_index, bool is_open);

    void        draw_reticule();

}


#endif  // _GFX_UTILS_HEADER_
