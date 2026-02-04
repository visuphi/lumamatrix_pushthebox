/*
 * Copyright (c) 2026 Institute of Embedded Systems ZHAW
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>

#include <map.h>
#include <buttons.h>


#define DELAY_TIME K_MSEC(CONFIG_SAMPLE_LED_UPDATE_DELAY)

#define RGB(_r, _g, _b) {.r = (_r), .g = (_g), .b = (_b)}




uint8_t map[8][8] = {{1, 1, 1, 1, 1, 0, 0, 0}, 
                    {1, 0, 2, 3, 1, 1, 1, 0},
                    {1, 0, 0, 3, 3, 0, 1, 0},
                    {1, 0, 0, 1, 1, 2, 1, 1},
                    {1, 1, 0, 0, 1, 0, 0, 1},
                    {0, 1, 2, 0, 0, 0, 0, 1},
                    {0, 1, 0, 0, 1, 1, 1, 1},
                    {0, 1, 1, 1, 1, 0, 0, 0}};

struct Position start = {.x = 2, .y = 2};

/* callbacks which are called when joystick is used */
void joystick_up_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick up callback");
	int rc;
	
	struct Position new_position = {.x = -1, .y = 0};
	rc = move_to(new_position);
	if (rc) {
		LOG_ERR("failed to move player");
	}
}

void joystick_down_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick down callback");
	int rc;

	struct Position new_position = {.x = 1, .y = 0};
	rc = move_to(new_position);
	if (rc) {
		LOG_ERR("failed to move player");
	}
}

void joystick_left_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick left callback");
	int rc;

	struct Position new_position = {.x = 0, .y = -1};
	rc = move_to(new_position);
	if (rc) {
		LOG_ERR("failed to move player");
	}
}

void joystick_right_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick right callback");
	int rc;

	struct Position new_position = {.x = 0, .y = 1};
	rc = move_to(new_position);
	if (rc) {
		LOG_ERR("failed to move player");
	}
}

void joystick_center_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick center callback");
	int rc;

	rc = load_map(map, start);
	if (rc) {
		LOG_ERR("failed to load map");
	}
}

/* HARDWARE INIT */
static int init_hw(void)
{
	int rc;

	rc = init_map();
	if (rc) {
		LOG_ERR("failed to init map");
	}

	rc = init_buttons(joystick_up_pressed, joystick_down_pressed, joystick_left_pressed,
			      joystick_right_pressed, joystick_center_pressed);
	if (rc) {
		LOG_ERR("failed to init buttons: %d", rc);
		return rc;
	}

	return 0;
}

int main(void)
{
	int rc;

	rc = init_hw();
	if (rc) {
		LOG_ERR("failed to init hw");
	}

	rc = load_map(map, start);
	if (rc) {
		LOG_ERR("failed to load map");
	}
	return 0;
}
