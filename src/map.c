#include <map.h>

int map_pos_to_index(int row, int col)
{
    return (7-row) * 8 + col;
}

// struct map maps[] = {
//     {
//         BORDER_CELL,
//         { 0, 0 },
//         { 100, 100, 100 }
//     },
//         {
//         BORDER_CELL,
//         { 1, 1 },
//         { 100, 100, 100 }
//     },
//         {
//         BORDER_CELL,
//         { 2, 0 },
//         { 100, 100, 100 }
//     },
//         {
//         BORDER_CELL,
//         { 3, 0 },
//         { 100, 100, 100 }
//     },
//         {
//         BORDER_CELL,
//         { 4, 0 },
//         { 100, 100, 100 }
//     },
//         {
//         BORDER_CELL,
//         { 5, 0 },
//         { 100, 100, 100 }
//     },
//         {
//         BORDER_CELL,
//         { 5, 1 },
//         { 100, 100, 100 }
//     },
//         {
//         BORDER_CELL,
//         { 5, 2 },
//         { 100, 100, 100 }
//     }
// };