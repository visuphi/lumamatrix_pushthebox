#include <map.h>
#include <zephyr/drivers/led_strip.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(map);

#define RGB(_r, _g, _b) {.r = (_r), .g = (_g), .b = (_b)}


static struct led_rgb pixels[STRIP_NUM_PIXELS];

uint8_t game_map[8][8];
uint8_t game_state[8][8];
struct Position player;

static const struct led_rgb colors[] = {
	RGB(CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00, 0x00), /* red */
	RGB(0x00, CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00), /* green */
	RGB(0x00, 0x00, CONFIG_SAMPLE_LED_BRIGHTNESS), /* blue */
};

static const struct led_rgb field_colors[] = {
	RGB(0, 0, 0),       /* empty */
	RGB(100, 100, 100), /* border */
	RGB(128, 0, 128),   /* BOX */
	RGB(0, 128, 128),   /* TARGET */
	RGB(0, 128, 0),     /* BOX_ON_TARGET */
};

struct led_rgb player_color = RGB(255, 0, 0);

int init_map(void) {
    return init_matrix();
}

int map_pos_to_index(int row, int col)
{
    return (7-row) * 8 + col;
}

int display_map(void)
{
	size_t map_len_y = sizeof(game_state[0]) / sizeof(game_state[0][0]);
	size_t map_len_x = sizeof(game_state) / sizeof(game_state[0]);
	int x, y;
	int index, element;
	for (y = 0; y < map_len_y; y++) {
		for (x = 0; x < map_len_x; x++) {
			index = map_pos_to_index(y, x);
			element = game_state[y][x];
			pixels[index] = field_colors[element];
		}
	}

	index = map_pos_to_index(player.x, player.y);
	pixels[index] = player_color;

    return display(pixels);
}

int display_win(void)
{
	size_t map_len_y = sizeof(game_state[0]) / sizeof(game_state[0][0]);
	size_t map_len_x = sizeof(game_state) / sizeof(game_state[0]);
	int x, y;
	int index;
	for (y = 0; y < map_len_y; y++) {
		for (x = 0; x < map_len_x; x++) {
			index = map_pos_to_index(y, x);
			pixels[index] = colors[1];
		}
	}

    return display(pixels);
}

int load_map(uint8_t new_map[8][8], struct Position start_pos)
{
	memcpy(game_state, new_map, sizeof(game_state));
	memcpy(game_map, new_map, sizeof(game_map));
	player = start_pos;
	
	return display_map();
}

bool check_success(void)
{
	size_t map_len_y = sizeof(game_state[0]) / sizeof(game_state[0][0]);
	size_t map_len_x = sizeof(game_state) / sizeof(game_state[0]);
	int x, y;
	for (y = 0; y < map_len_y; y++) {
		for (x = 0; x < map_len_x; x++) {
			if (game_state[y][x] == BOX) {
				return false;
			}
		}
	}
	return true;
}

int move_to(struct Position rel_pos)
{
	int new_x = player.x + rel_pos.x;
	int new_y = player.y + rel_pos.y;

	switch (game_state[new_x][new_y]) {
	case BORDER:
		break;
	case BOX_ON_TARGET:
		switch (game_state[new_x + rel_pos.x][new_y + rel_pos.y]) {
		case BORDER:        // intentional fallthrough
		case BOX_ON_TARGET: // intentional fallthrough
		case BOX:
			break;
		case NOTHING:
			game_state[new_x][new_y] = TARGET;
			game_state[new_x + rel_pos.x][new_y + rel_pos.y] = BOX;
			player.x = new_x;
			player.y = new_y;
			break;
		case TARGET:
			game_state[new_x][new_y] = TARGET;
			game_state[new_x + rel_pos.x][new_y + rel_pos.y] = BOX_ON_TARGET;
			player.x = new_x;
			player.y = new_y;
			break;
		}
		break;
	case BOX:
		switch (game_state[new_x + rel_pos.x][new_y + rel_pos.y]) {
		case BORDER:        // intentional fallthrough
		case BOX_ON_TARGET: // intentional fallthrough
		case BOX:
			break;
		case NOTHING:
			game_state[new_x][new_y] = NOTHING;
			game_state[new_x + rel_pos.x][new_y + rel_pos.y] = BOX;
			player.x = new_x;
			player.y = new_y;
			break;
		case TARGET:
			game_state[new_x][new_y] = NOTHING;
			game_state[new_x + rel_pos.x][new_y + rel_pos.y] = BOX_ON_TARGET;
			player.x = new_x;
			player.y = new_y;
			break;
		}
		break;
	case TARGET: // intentional fallthrough
	default:     // intentional fallthrough
	case NOTHING:
		player.x = new_x;
		player.y = new_y;
		break;
	}

	if (check_success()) {
		return display_win();
	} else {
		return display_map();
	}
}