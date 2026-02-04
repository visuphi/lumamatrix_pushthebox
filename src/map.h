#include <matrix.h>

#define NOTHING       0
#define BORDER        1
#define BOX           2
#define TARGET        3
#define BOX_ON_TARGET 4

struct Position {
    int x;
    int y;
} ;


struct map {
    int name;
    struct Position position;
    struct led_rgb color;
};

int init_map(void);
int load_map(uint8_t new_map[8][8], struct Position start_pos);
int move_to(struct Position rel_pos);