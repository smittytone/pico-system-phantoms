/*
 * Phantom Slayer
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _GFX_UTILS_HEADER_
#define _GFX_UTILS_HEADER_

using namespace picosystem;


/*
 * ENUMERATIONS
 */
// Indices for the textual sprites
enum class WORDS: uint8_t {
    SCORE = 0,
    OVER,
    KILLS,
    HIGH,
    HITS,
    GAME,
    NEW,
    LEVEL,
    ANY_KEY,
    PLAYER_DEAD
};

// Colours
enum class COLOURS: color_t {
    // G.B.A.R
    GREEN = 0xF0F0,
    BLUE = 0x0FF0,
    RED = 0x00FF,
    WHITE = 0xFFFF,
    YELLOW = 0xF0FF,
    ORANGE = 0xA0FF,
    CLEAR = 0x0000,
    BLACK = 0x00F0
};


/*
 * PROTOTYPES
 */
namespace Gfx {
    void        draw_screen(uint8_t x, uint8_t y, uint8_t direction);
    bool        draw_section(uint8_t x, uint8_t y, uint8_t left_dir, uint8_t right_dir,
                             uint8_t current_frame, uint8_t furthest_frame);
    void        draw_floor_line(uint8_t frame_index);
    void        draw_teleporter(uint8_t frame_index);
    void        draw_left_wall(uint8_t frame_index, bool is_open);
    void        draw_right_wall(uint8_t frame_index, bool is_open);
    void        draw_far_wall(uint8_t frame_index);

    void        draw_reticule(void);
    void        draw_zap(uint8_t frame);
    void        animate_turn(void);
    void        draw_phantom(uint8_t frame_number, uint8_t* phantom_count, bool is_zapped);

    void        draw_word(WORDS index, uint8_t x, uint8_t y, bool do_double);
    void        draw_number(uint8_t number, uint8_t x, uint8_t y, bool do_double = false);

    void        animate_credit(int16_t y);
    void        animate_logo(int16_t y);

    //void      show_debug_info(void);

    void        alt_blit(const buffer_t *src, int32_t sx, int32_t sy, int32_t w, int32_t h, int32_t dx, int32_t dy);
    void        cls(COLOURS colour);
}


/*
 * EXTERNALLY-DEFINED GLOBALS
 */
extern const uint16_t   phantom_sprites[];
extern const uint16_t   zapped_sprites[];
extern const uint8_t    phantom_sizes[];
extern const uint8_t    word_sizes[];
extern const uint16_t   word_sprites[];
extern const uint16_t   logo_sprite[];
extern const uint16_t   credit_sprite[];


#endif  // _GFX_UTILS_HEADER_
