#include <map.h>

int map_pos_to_index(int row, int col)
{
    return (7-row) * 8 + col;
}
