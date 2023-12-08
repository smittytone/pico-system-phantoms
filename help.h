/*
 * Phantom Slayer
 * Help page display
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _HELP_PAGES_HEADER_
#define _HELP_PAGES_HEADER_


/*
 * CONSTANTS
 */
constexpr uint8_t MAX_HELP_PAGES = 5;


/*
 * PROTOTYPES
 */
namespace Help {
    void        show_page(uint16_t page_number);
    void        show_offer(void);
}


#endif  // _HELP_PAGES_HEADER_
