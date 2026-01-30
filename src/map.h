#include <zephyr/drivers/led_strip.h>

struct Position {
    int x;
    int y;
} ;


struct map {
    int name;
    struct Position position;
    struct led_rgb color;
};

int map_pos_to_index(int row, int col);