/*
 * Phantom Slayer
 * Help page display
 *
 * @version     1.1.2
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#ifndef _HELP_PAGES_HEADER_
#define _HELP_PAGES_HEADER_


/*
 *      CONSTANTS
 */
#define MAX_HELP_PAGES          5


/*
 *      PROTOTYPES
 */
namespace Help {
    void        show_page(uint16_t page_number);
    void        show_offer();
}


#endif  // _HELP_PAGES_HEADER_