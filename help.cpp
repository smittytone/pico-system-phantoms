/*
 * Phantom Slayer
 * Help page display
 *
 * @version     1.1.0
 * @author      smittytone
 * @copyright   2021, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using std::string;
using std::vector;
using namespace picosystem;


/*
 *      HELP PAGE STRINGS
 */
string help_page_0 =
"PHANTOM SLAYER is a chase game played in a\n"
"three-dimensional maze. Each maze is haunted\n"
"by evil Phantoms which can destroy you with a\n"
"single touch.\n\n"
"YOUR MISSION Destroy them before they\n"
"destroy you!\n\n"
"The graphics in PHANTOM SLAYER are a\n"
"three-dimensional representation.\n\n"
"You are occasionally given the opportunity\n"
"to view the maze from above, but usually you\n"
"must operate from inside it. Look into your\n"
"monitor as though you were looking down a\n"
"hallway. To move forward one step, press the\n"
"up arrow. The right and left arrows turn you\n"
"to the right and left. The down arrow moves\n"
"you one step backwards.\n\n"
"Moving and turning produce smoothly moving\n"
"graphics on the screen, showing you a view of\n"
"the maze as you move or turn.";

string help_page_1 =
"Your enemies in the maze are the Phantoms.\n"
"If a Phantom touches you, you are destroyed.\n"
"They always know where you are and will try to\n"
"reach you by the most direct path. You destroy\n"
"Phantoms with your laser pistol.\n\n"
"It may take more than one hit from your pistol\n"
"to kill a Phantom. When one Phantom is\n"
"destroyed, another is created to take its place.\n"
"There are never more than three Phantoms in\n"
"the maze at once. As the game progresses,\n"
"the Phantoms become harder to destroy.\n"
"At the beginning, a Phantom never takes\n"
"more than three hits, but later they may\n"
"require more.\n\n"
"Once a Phantom is destroyed, you have a\n"
"chance to look at the maze from above for a\n"
"few seconds. You also see this view at the\n"
"beginning of the game. In this map, the\n"
"Phantoms are denoted by red dots.\n"
"You are the arrow. The maze is a 20 x 20 grid,\n"
"and you begin the game near its centre.";

string help_page_2 =
"Your weapons are a laser pistol and a Phantom\n"
"detector. Your pistol is 'armed' by pressing\n"
"the 'A' key. Releasing it fires. Arming\n"
"causes a cross-hair sight to appear,\n"
"automatically centered on the target.\n"
"Once your pistol is armed, you cannot move or\n"
"turn until it is fired.\n\n"
"IMPORTANT It takes 2 seconds to recharge\n"
"your pistol after it has been fired.\n"
"The Phantom detector is an audio tone\n"
"triggered by nearby Phantoms. You select\n"
"the triggering distance by pressing the\n"
"'X' (up) and 'Y' (down) keys.\n\n"
"It is initially set at 4. The Phantom detector\n"
"is triggered by Phantoms at the distance you\n"
"set, regardless of intervening walls.\n\n"
"By changing the triggering distance, you can\n"
"get an accurate reading of the distance\n"
"between you and the Phantom.";

string help_page_3 =
"The green square can be a very valuable\n"
"escape route. It is randomly placed in the \n"
"maze at the start of the game. If you stand\n"
"on it, you can return to your starting point\n"
"in the game by pressing the 'B' key.\n\n"
"Once you have used a green square,\n"
"it disappears and a new one is generated\n"
"somewhere else in the maze. The green square\n"
"is not shown in the maze map, except at the\n"
"end of the game.\n\n"
"You score 2 points for each hit and 10 points\n"
"for each 'kill'. At the end of each game (i.e.\n"
"at your death) the map and the score are\n"
"displayed.";

string help_page_4 =
"STRATEGY Since it may take more than one hit\n"
"to kill a Phantom, and your pistol takes time\n"
"to recharge, you must learn to fire, turn and\n"
"run from Phantoms.\n\n"
"You will find that certain arrangements of\n"
"hallways and corners are best for this tactic,\n"
"and that other arrangements are perfect\n"
"spots for a Phantom ambush on you.\n\n"
"Remember the green square as an escape\n"
"route.\n\n";


/*
 *      GLOBALS
 */
vector<string> help_pages;


namespace Help {


/*
    Load the help strings into a page array.
 */
void init() {
    help_pages.push_back(help_page_0);
    help_pages.push_back(help_page_1);
    help_pages.push_back(help_page_2);
    help_pages.push_back(help_page_3);
    help_pages.push_back(help_page_4);
}


/*
    Show a page of help text.

    - Parameters:
        - page_number: The index of the page to display.
 */
void show_page(uint16_t page_number) {
    // CLS
    pen(0, 15, 0);
    clear();

    // Game title
    pen(0, 0, 0);
    cursor(80, 10);
    text("PHANTOM SLAYER");

    // Show the help text
    cursor(10, 30);
    text(help_pages.at(page_number));

    // Call to action
    cursor(47, 230);
    text("PRESS ANY KEY TO CONTINUE");
}


/*
    Ask if the player wants help.
 */
void show_offer() {
    // CLS
    pen(0, 15, 0);
    clear();

    // Game title
    pen(0, 0, 0);
    cursor(80, 20);
    text("PHANTOM SLAYER");

    cursor(81, 115);
    text("INSTRUCTIONS?");

    // Call to action
    cursor(78, 200);
    text("PRESS 'A' FOR HELP,");
    cursor(53, 210);
    text("OR ANY OTHER KEY TO PLAY");
}


}   // namespace Help