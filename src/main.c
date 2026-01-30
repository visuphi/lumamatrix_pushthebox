/*
 * Copyright (c) 2026 Institute of Embedded Systems ZHAW
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <map.h>
#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

#define STRIP_NODE DT_ALIAS(led_strip)

#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

#define DELAY_TIME K_MSEC(CONFIG_SAMPLE_LED_UPDATE_DELAY)

#define RGB(_r, _g, _b) {.r = (_r), .g = (_g), .b = (_b)}

#define NOTHING         0
#define BORDER          1
#define BOX             2
#define TARGET          3
#define BOX_ON_TARGET   4

static const struct led_rgb field_colors[] = {
	RGB(0x00, 0x00, 0x00), /* empty */
	RGB(CONFIG_SAMPLE_LED_BRIGHTNESS, CONFIG_SAMPLE_LED_BRIGHTNESS, CONFIG_SAMPLE_LED_BRIGHTNESS), /* border */
	RGB(CONFIG_SAMPLE_LED_BRIGHTNESS, CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00), /* BOX */
	RGB(CONFIG_SAMPLE_LED_BRIGHTNESS,  0x00, CONFIG_SAMPLE_LED_BRIGHTNESS), /* TARGET */
	RGB(0x00, CONFIG_SAMPLE_LED_BRIGHTNESS, CONFIG_SAMPLE_LED_BRIGHTNESS), /* BOX_ON_TARGET */
};

static const struct gpio_dt_spec joystick_up = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec joystick_down = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const struct gpio_dt_spec joystick_left = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const struct gpio_dt_spec joystick_right = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);
static const struct gpio_dt_spec joystick_center = GPIO_DT_SPEC_GET(DT_ALIAS(sw4), gpios);
static const struct gpio_dt_spec switcher = GPIO_DT_SPEC_GET(DT_ALIAS(sw5), gpios);

static struct gpio_callback joystick_up_cb_data;
static struct gpio_callback joystick_down_cb_data;
static struct gpio_callback joystick_left_cb_data;
static struct gpio_callback joystick_right_cb_data;
static struct gpio_callback joystick_center_cb_data;


uint8_t map[8][8] = {{1, 1, 1, 1, 1, 0, 0, 0},
                    {1, 0, 2, 3, 1, 1, 1, 0},
                    {1, 0, 0, 3, 3, 0, 1, 0},
                    {1, 0, 0, 1, 1, 2, 1, 1},
                    {1, 1, 0, 0, 1, 0, 0, 1},
                    {0, 1, 2, 0, 0, 0, 0, 1},
                    {0, 1, 0, 0, 1, 1, 1, 1},
                    {0, 1, 1, 1, 1, 0, 0, 0}};

struct Position start = {.x = 2, .y = 2};
struct led_rgb player_color = RGB(CONFIG_SAMPLE_LED_BRIGHTNESS/2, CONFIG_SAMPLE_LED_BRIGHTNESS, CONFIG_SAMPLE_LED_BRIGHTNESS/2);

uint8_t game_state[8][8];
struct Position player;

int cursor = 0;
static const struct led_rgb colors[] = {
	RGB(CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00, 0x00), /* red */
	RGB(0x00, CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00), /* green */
	RGB(0x00, 0x00, CONFIG_SAMPLE_LED_BRIGHTNESS), /* blue */
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

static int display_map(void) {
	int rc;
	size_t map_len_y = sizeof(game_state[0]) / sizeof(game_state[0][0]);
	size_t map_len_x = sizeof(game_state) / sizeof(game_state[0]);
	int x, y;
	int index, element;
	for (y=0; y < map_len_y; y++) {
	    for (x=0; x < map_len_x; x++) {
            index = map_pos_to_index(y,x);
            element = game_state[y][x];
            pixels[index] = field_colors[element];
        }
	}

    index = map_pos_to_index(player.x,player.y);
    pixels[index] = player_color;

	rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
	if (rc) {
		LOG_ERR("couldn't update strip: %d", rc);
	}
	return 0;
}

static int display_win(void) {
	int rc;
	size_t map_len_y = sizeof(game_state[0]) / sizeof(game_state[0][0]);
	size_t map_len_x = sizeof(game_state) / sizeof(game_state[0]);
	int x, y;
	int index;
	for (y=0; y < map_len_y; y++) {
	    for (x=0; x < map_len_x; x++) {
            index = map_pos_to_index(y,x);
            pixels[index] = colors[1];
        }
	}

	rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
	if (rc) {
		LOG_ERR("couldn't update strip: %d", rc);
	}
	return 0;
}

static int load_map(void) {
    memcpy(game_state, map, sizeof(map));
    player = start;
	return 0;
}

bool check_success(void) {
	size_t map_len_y = sizeof(game_state[0]) / sizeof(game_state[0][0]);
	size_t map_len_x = sizeof(game_state) / sizeof(game_state[0]);
	int x, y;
	for (y=0; y < map_len_y; y++) {
	    for (x=0; x < map_len_x; x++) {
            if (game_state[y][x] == BOX) {
                return false;
            }
        }
	}
    return true;
}


void move_to(struct Position rel_pos) {
    int new_x = player.x + rel_pos.x;
    int new_y = player.y + rel_pos.y;

    switch (game_state[new_x][new_y]) {
        case BORDER:
            break;
        case BOX_ON_TARGET:
            switch (game_state[new_x + rel_pos.x][new_y + rel_pos.y]) {
                case BORDER: // intentional fallthrough
                case BOX_ON_TARGET:  // intentional fallthrough
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
                case BORDER: // intentional fallthrough
                case BOX_ON_TARGET:  // intentional fallthrough
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
        default: // intentional fallthrough
        case NOTHING:
            player.x = new_x;
            player.y = new_y;
            break;
    }
    
    if (check_success()) {
        display_win();
    }
    else {
        display_map();
    }
}

/* callbacks which are calle when joystick is used */
void joystick_up_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick up callback");
    struct Position new_position = {.x = -1, .y = 0};
    move_to(new_position);
}

void joystick_down_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick down callback");
    struct Position new_position = {.x = 1, .y = 0};
    move_to(new_position);
}

void joystick_left_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick left callback");
    struct Position new_position = {.x = 0, .y = -1};
    move_to(new_position);
}

void joystick_right_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick right callback");
    struct Position new_position = {.x = 0, .y = 1};
    move_to(new_position);
}

void joystick_center_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    load_map();
    display_map();
	LOG_INF("joystick center callback");
}

/* helper to  set up the gpio/joystick input */
static int setup_button(const struct gpio_dt_spec *btn, struct gpio_callback *cb,
			gpio_callback_handler_t handler, bool enable_irq)
{
	int rc;

	if (!gpio_is_ready_dt(btn)) {
		printk("Error: button device %s is not ready\n", btn->port->name);
		return -1;
	}

	rc = gpio_pin_configure_dt(btn, GPIO_INPUT);
	if (rc != 0) {
		printk("Error %d: failed to configure %s pin %d\n", rc, btn->port->name, btn->pin);
		return -1;
	}

	if (enable_irq) {
		rc = gpio_pin_interrupt_configure_dt(btn, GPIO_INT_EDGE_TO_ACTIVE);
		if (rc != 0) {
			printk("Error %d: failed to configure interrupt on %s pin %d\n", rc,
			       btn->port->name, btn->pin);
			return -1;
		}
	}
    /* set callbacks */
	gpio_init_callback(cb, handler, BIT(btn->pin));
	gpio_add_callback(btn->port, cb);

	return 0;
}



/* function to initialise the  hardware */
static int init_hw(void)
{
	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return -1;
	}

	if (setup_button(&joystick_up, &joystick_up_cb_data, joystick_up_pressed, true)) {
		return -1;
	}

	if (setup_button(&joystick_down, &joystick_down_cb_data, joystick_down_pressed, true)) {
		return -1;
	}

	if (setup_button(&joystick_left, &joystick_left_cb_data, joystick_left_pressed, true)) {
		return -1;
	}

	if (setup_button(&joystick_right, &joystick_right_cb_data, joystick_right_pressed, true)) {
		return -1;
	}

	if (setup_button(&joystick_center, &joystick_center_cb_data, joystick_center_pressed,
			 true)) {
		return -1;
	}

	return 0;
}

int main(void)
{

	int rc;

	rc = init_hw();
	if (!rc) {
		LOG_ERR("failed to init hw");
	}

    rc = load_map();

	rc = display_map();


	return 0;
}
