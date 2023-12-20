/*
 * Phantom Slayer
 * Map management functions
 *
 * @author      smittytone
 * @copyright   2023, Tony Smith
 * @licence     MIT
 *
 */
#include "main.h"

using namespace picosystem;


/*
 *      EXTERNALLY-DEFINED GLOBALS
 */
extern Game     game;


/*
 *      GLOBALS
 */
uint8_t         *current_map[20];

/*
 *      MAP DATA
 */
uint8_t base_map_00[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_01[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE};
uint8_t base_map_02[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};
uint8_t base_map_03[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_04[20] = {0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_05[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE};
uint8_t base_map_06[20] = {0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_07[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE};
uint8_t base_map_08[20] = {0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_09[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_10[20] = {0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_11[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE};
uint8_t base_map_12[20] = {0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_13[20] = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF};
uint8_t base_map_14[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_15[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE};
uint8_t base_map_16[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};
uint8_t base_map_17[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_18[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE};
uint8_t base_map_19[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};

uint8_t base_map_20[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_21[20] = {0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_22[20] = {0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_23[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_24[20] = {0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_25[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_26[20] = {0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};
uint8_t base_map_27[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF};
uint8_t base_map_28[20] = {0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE};
uint8_t base_map_29[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_30[20] = {0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_31[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF};
uint8_t base_map_32[20] = {0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE};
uint8_t base_map_33[20] = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF};
uint8_t base_map_34[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};
uint8_t base_map_35[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF};
uint8_t base_map_36[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_37[20] = {0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_38[20] = {0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_39[20] = {0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};

uint8_t base_map_40[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_41[20] = {0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};
uint8_t base_map_42[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};
uint8_t base_map_43[20] = {0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_44[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_45[20] = {0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_46[20] = {0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_47[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_48[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_49[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_50[20] = {0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF};
uint8_t base_map_51[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_52[20] = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_53[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE};
uint8_t base_map_54[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF};
uint8_t base_map_55[20] = {0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF};
uint8_t base_map_56[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_57[20] = {0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};
uint8_t base_map_58[20] = {0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE};
uint8_t base_map_59[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t base_map_60[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_61[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE};
uint8_t base_map_62[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_63[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_64[20] = {0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF};
uint8_t base_map_65[20] = {0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_66[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF};
uint8_t base_map_67[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF};
uint8_t base_map_68[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF};
uint8_t base_map_69[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE};
uint8_t base_map_70[20] = {0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF};
uint8_t base_map_71[20] = {0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_72[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_73[20] = {0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_74[20] = {0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE};
uint8_t base_map_75[20] = {0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_76[20] = {0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE};
uint8_t base_map_77[20] = {0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_78[20] = {0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_79[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t base_map_80[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_81[20] = {0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE};
uint8_t base_map_82[20] = {0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_83[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_84[20] = {0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_85[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};
uint8_t base_map_86[20] = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_87[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE};
uint8_t base_map_88[20] = {0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};
uint8_t base_map_89[20] = {0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_90[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE};
uint8_t base_map_91[20] = {0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_92[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_93[20] = {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_94[20] = {0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF};
uint8_t base_map_95[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_96[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_97[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};
uint8_t base_map_98[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE};
uint8_t base_map_99[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};

uint8_t base_map_100[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_101[20] = {0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE};
uint8_t base_map_102[20] = {0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF};
uint8_t base_map_103[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_104[20] = {0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_105[20] = {0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_106[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF};
uint8_t base_map_107[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_108[20] = {0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_109[20] = {0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_110[20] = {0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_111[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF};
uint8_t base_map_112[20] = {0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE};
uint8_t base_map_113[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_114[20] = {0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_115[20] = {0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t base_map_116[20] = {0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xFF};
uint8_t base_map_117[20] = {0xFF, 0xEE, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF};
uint8_t base_map_118[20] = {0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF, 0xEE, 0xFF, 0xEE, 0xEE, 0xFF, 0xEE, 0xEE, 0xEE, 0xFF};
uint8_t base_map_119[20] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


namespace Map {


/**
 * @brief Select and populate a new level's map.
 *
 * @param last_map: The previous map, provided so it's
 *                  not selected again.
 */
uint8_t init(uint8_t last_map) {

    // FROM 1.0.2
    // Don't pick the same map as last time
    uint8_t map;
    do {
        map = Utils::irandom(0, NUMBER_OF_MAPS);
    } while (map == last_map);

    /* TEST VALUE
    map = 1;
    */

    switch(map) {
        case 0:
            current_map[0] = base_map_00;
            current_map[1] = base_map_01;
            current_map[2] = base_map_02;
            current_map[3] = base_map_03;
            current_map[4] = base_map_04;
            current_map[5] = base_map_05;
            current_map[6] = base_map_06;
            current_map[7] = base_map_07;
            current_map[8] = base_map_08;
            current_map[9] = base_map_09;
            current_map[10] = base_map_10;
            current_map[11] = base_map_11;
            current_map[12] = base_map_12;
            current_map[13] = base_map_13;
            current_map[14] = base_map_14;
            current_map[15] = base_map_15;
            current_map[16] = base_map_16;
            current_map[17] = base_map_17;
            current_map[18] = base_map_18;
            current_map[19] = base_map_19;
            break;
        case 1:
            current_map[0] = base_map_40;
            current_map[1] = base_map_41;
            current_map[2] = base_map_42;
            current_map[3] = base_map_43;
            current_map[4] = base_map_44;
            current_map[5] = base_map_45;
            current_map[6] = base_map_46;
            current_map[7] = base_map_47;
            current_map[8] = base_map_48;
            current_map[9] = base_map_49;
            current_map[10] = base_map_50;
            current_map[11] = base_map_51;
            current_map[12] = base_map_52;
            current_map[13] = base_map_53;
            current_map[14] = base_map_54;
            current_map[15] = base_map_55;
            current_map[16] = base_map_56;
            current_map[17] = base_map_57;
            current_map[18] = base_map_58;
            current_map[19] = base_map_59;
            break;
        case 2:
            current_map[0] = base_map_60;
            current_map[1] = base_map_61;
            current_map[2] = base_map_62;
            current_map[3] = base_map_63;
            current_map[4] = base_map_64;
            current_map[5] = base_map_65;
            current_map[6] = base_map_66;
            current_map[7] = base_map_67;
            current_map[8] = base_map_68;
            current_map[9] = base_map_69;
            current_map[10] = base_map_70;
            current_map[11] = base_map_71;
            current_map[12] = base_map_72;
            current_map[13] = base_map_73;
            current_map[14] = base_map_74;
            current_map[15] = base_map_75;
            current_map[16] = base_map_76;
            current_map[17] = base_map_77;
            current_map[18] = base_map_78;
            current_map[19] = base_map_79;
            break;
        case 3:
            current_map[0] = base_map_80;
            current_map[1] = base_map_81;
            current_map[2] = base_map_82;
            current_map[3] = base_map_83;
            current_map[4] = base_map_84;
            current_map[5] = base_map_85;
            current_map[6] = base_map_86;
            current_map[7] = base_map_87;
            current_map[8] = base_map_88;
            current_map[9] = base_map_89;
            current_map[10] = base_map_90;
            current_map[11] = base_map_91;
            current_map[12] = base_map_92;
            current_map[13] = base_map_93;
            current_map[14] = base_map_94;
            current_map[15] = base_map_95;
            current_map[16] = base_map_96;
            current_map[17] = base_map_97;
            current_map[18] = base_map_98;
            current_map[19] = base_map_99;
            break;
        case 4:
            current_map[0] = base_map_100;
            current_map[1] = base_map_101;
            current_map[2] = base_map_102;
            current_map[3] = base_map_103;
            current_map[4] = base_map_104;
            current_map[5] = base_map_105;
            current_map[6] = base_map_106;
            current_map[7] = base_map_107;
            current_map[8] = base_map_108;
            current_map[9] = base_map_109;
            current_map[10] = base_map_110;
            current_map[11] = base_map_111;
            current_map[12] = base_map_112;
            current_map[13] = base_map_113;
            current_map[14] = base_map_114;
            current_map[15] = base_map_115;
            current_map[16] = base_map_116;
            current_map[17] = base_map_117;
            current_map[18] = base_map_118;
            current_map[19] = base_map_119;
            break;
        default:
            current_map[0] = base_map_20;
            current_map[1] = base_map_21;
            current_map[2] = base_map_22;
            current_map[3] = base_map_23;
            current_map[4] = base_map_24;
            current_map[5] = base_map_25;
            current_map[6] = base_map_26;
            current_map[7] = base_map_27;
            current_map[8] = base_map_28;
            current_map[9] = base_map_29;
            current_map[10] = base_map_30;
            current_map[11] = base_map_31;
            current_map[12] = base_map_32;
            current_map[13] = base_map_33;
            current_map[14] = base_map_34;
            current_map[15] = base_map_35;
            current_map[16] = base_map_36;
            current_map[17] = base_map_37;
            current_map[18] = base_map_38;
            current_map[19] = base_map_39;
    }

    return map;
}


/**
 * @brief Draw the current map on the screen buffer, centred but
 *        vertically adjusted according to `y_delta`.
 *        If `show_entities` is `true`, the phantom locations
 *        are plotted in. The player and the teleport sqaure positions
 *        are always shown.
 *
 *        NOTE With the map now drawn on 3x3 blocks, `y_delta`
 *             has s very limited range of useable values.
 *
 * @param y_delta:       Offset in the y-axis.
 * @param show_entities: Display phantoms.
 */
void draw(uint8_t y_delta, bool show_entities, bool show_tele) {

    // Set the map background (blue)
    // NOTE Assumes caller has already run `cls()`
    pen((color_t)COLOURS::BLUE);
    frect(0, 40, 200, 160);

    // Draw the map
    uint8_t x = 40;
    uint8_t y = 40 + y_delta;

    for (uint8_t i = 0 ; i < 20 ; ++i) {
        const uint8_t *line = current_map[i];
        for (uint8_t j = 0 ; j < 20 ; ++j) {
            uint8_t pixel = line[j];

            // Draw and empty (path) square
            if (pixel != MAP_TILE_WALL) {
                pen((color_t)COLOURS::YELLOW);

                if (i == game.tele_y && j == game.tele_x && show_tele) {
                    // Show the teleport square in green
                    pen((color_t)COLOURS::GREEN);
                }

                if (show_entities) {
                    // Show any phantoms at the current square as a red square
                    for (auto p: game.phantoms) {
                        if (j == p.x && i == p.y) {
                            pen((color_t)COLOURS::RED);
                        }
                    }
                }

                frect(x + j * 8, y + i * 8, 8, 8);
            }

            // Show the player as an arrow at the current square
            if (j == game.player.x && i == game.player.y) {
                pen((color_t)COLOURS::RED);
                switch(game.player.direction) {
                    case DIRECTION::NORTH:
                        frect(x + j * 8 + 3, y + i * 8, 2, 3);
                        frect(x + j * 8, y + i * 8 + 3, 8, 2);
                        frect(x + j * 8, y + i * 8 + 5, 2, 3);
                        frect(x + j * 8 + 6, y + i * 8 + 5, 2, 3);
                        break;
                    case DIRECTION::EAST:
                        frect(x + j * 8, y + i * 8, 3, 2);
                        frect(x + j * 8, y + i * 8 + 6, 3, 2);
                        frect(x + j * 8 + 3, y + i * 8, 2, 8);
                        frect(x + j * 8 + 5, y + i * 8 + 3, 3, 2);
                        break;
                    case DIRECTION::SOUTH:
                        frect(x + j * 8 + 3, y + i * 8 + 5, 2, 3);
                        frect(x + j * 8, y + i * 8 + 3, 8, 2);
                        frect(x + j * 8, y + i * 8, 2, 3);
                        frect(x + j * 8 + 6, y + i * 8, 2, 3);
                        break;
                    default:
                        frect(x + j * 8 + 5, y + i * 8, 3, 2);
                        frect(x + j * 8 + 5, y + i * 8 + 6, 3, 2);
                        frect(x + j * 8 + 3, y + i * 8, 2, 8);
                        frect(x + j * 8, y + i * 8 + 3, 3, 2);
                       break;
                }
            }
        }
    }
}


/**
 * @brief Return the contents of the specified grid reference.
 *
 * @param x: The square's x co-ordinate.
 * @param y: The square's y co-ordinate.
 *
 * @returns: The contents of the square.
 */
uint8_t get_square_contents(uint8_t x, uint8_t y) {

    if (x > MAP_MAX || y > MAP_MAX) return MAP_TILE_WALL;
    const uint8_t *line = current_map[y];
    return line[x];
}


/**
 * @brief Set the contents of the specified grid reference.
 *
 * @param x:     The square's x co-ordinate.
 * @param y:     The square's y co-ordinate.
 * @param value: The square's new contents.
 *
 * @returns: `true` if the square was set, otherwise `false`.
 */
bool set_square_contents(uint8_t x, uint8_t y, uint8_t value) {

    if (x > MAP_MAX || y > MAP_MAX) return false;
    uint8_t *line = current_map[y];
    line[x] = value;
    return true;
}


/**
 * @brief Return the number of squares an entity can see.
 *
 * @param x:         The entity's x co-ordinate.
 * @param y:         The entity's y co-ordinate.
 * @param direction: The direction in which the entity is facing.
 *
 * @returns: The number of visible squares up to a maximum,
 *           excluding the entity's square.
 */
uint8_t get_view_distance(int8_t x, int8_t y, uint8_t direction) {

    uint8_t count = 0;
    switch((DIRECTION)direction) {
        case DIRECTION::NORTH:
            if (y == 0) return count;
            --y;
            do {
                if (get_square_contents(x, y) == MAP_TILE_WALL) break;
                ++count;
                --y;
            } while (y >= 0);
            break;
        case DIRECTION::EAST:
            if (x > 18) return count;
            x++;
            do {
                if (get_square_contents(x, y) == MAP_TILE_WALL) break;
                ++count;
                ++x;
            } while (x < 20);
            break;
        case DIRECTION::SOUTH:
            if (y > 18) return count;
            ++y;
            do {
                if (get_square_contents(x, y) == MAP_TILE_WALL) break;
                ++count;
                ++y;
            } while (y < 20);
            break;
        default:
            if (x == 0) return count;
            --x;
            do {
                if (get_square_contents(x, y) == MAP_TILE_WALL) break;
                ++count;
                --x;
            } while (x >= 0);
    }

    if (count > MAX_VIEW_RANGE) count = MAX_VIEW_RANGE;
    return count;
}


/**
 * @brief Is there a Phantom on the specified square?
 *
 * @returns: The index of the Phantom in the vector,
 *           or `NONE` if the square is empty.
 */
uint8_t phantom_on_square(uint8_t x, uint8_t y) {

    for (size_t i = 0 ; i < game.phantoms.size() ; ++i) {
        const Phantom &p = game.phantoms.at(i);
        if (x == p.x && y == p.y) return ((uint8_t)i & 0x0F);
    }

    return NONE;
}


}   // namespace Map
