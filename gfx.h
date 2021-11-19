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


using namespace picosystem;

/*
 *      CONSTANTS
 */
#define WORD_SCORE              0
#define WORD_OVER               1
#define WORD_KILLS              2
#define WORD_HIGH               3
#define WORD_HITS               4
#define WORD_GAME               5
#define WORD_NEW                6
#define WORD_LEVEL              7
#define PHRASE_ANY_KEY          8
#define PHRASE_PLAYER_DEAD      9


/*
 *      PROTOTYPES
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

    void        draw_reticule();
    void        draw_zap(uint8_t frame);
    void        animate_turn();
    void        draw_phantom(uint8_t frame_number, uint8_t* phantom_count, bool is_zapped);

    void        draw_word(uint8_t index, uint8_t x, uint8_t y, bool do_double);
    void        draw_number(uint8_t number, uint8_t x, uint8_t y, bool do_double = false);

    void        animate_credit(int16_t y);
    void        animate_logo(int16_t y);

    void        show_debug_info();

    void        alt_blit(buffer_t *src, int32_t sx, int32_t sy, int32_t w, int32_t h, int32_t dx, int32_t dy);
}


/*
 *      EXTERNALLY-DEFINED GLOBALS
 */
extern const uint16_t   phantom_sprites[];
extern const uint16_t   zapped_sprites[];
extern const uint8_t    phantom_sizes[];
extern const uint8_t    word_sizes[];
extern const uint16_t   word_sprites[];
extern const uint16_t   logo_sprite[];
extern const uint16_t   credit_sprite[];


#endif  // _GFX_UTILS_HEADER_
