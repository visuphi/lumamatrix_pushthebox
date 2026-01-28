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

/* just for test purpose, can be done more efficient */
struct map maps[] = {
    {
        BORDER_CELL,
        { 1, 0 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 2, 0 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 3, 0 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 4, 0 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 4, 1 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 5, 1 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 6, 1 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 7, 1 },
        { 100, 100, 100 }
    },

    {
        BORDER_CELL,
        { 7, 2 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 7, 3 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 7, 4 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 6, 4 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 6, 5 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 6, 6 },
        { 100, 100, 100 }
    },

    {
        BORDER_CELL,
        { 5, 6 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 4, 6 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 4, 7 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 3, 7 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 2, 7 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 1, 7 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 0, 7 },
        { 100, 100, 100 }
    },

    {
        BORDER_CELL,
        { 0, 6 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 0, 5 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 0, 4 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 0, 3 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 1, 3 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 1, 2 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 1, 1 },
        { 100, 100, 100 }
    },

        {
        BORDER_CELL,
        { 3, 4 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 4, 4 },
        { 100, 100, 100 }
    },
        {
        BORDER_CELL,
        { 4, 3 },
        { 100, 100, 100 }
    }

};

int cursor = 0;
static const struct led_rgb colors[] = {
	RGB(CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00, 0x00), /* red */
	RGB(0x00, CONFIG_SAMPLE_LED_BRIGHTNESS, 0x00), /* green */
	RGB(0x00, 0x00, CONFIG_SAMPLE_LED_BRIGHTNESS), /* blue */
};

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
size_t color = 0;

/* callbacks which are calle when joystick is used */
void joystick_up_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int rc;
	if (cursor != 64) {
		cursor++;
	} else {
		cursor = 0;
	}
	memset(&pixels, 0x00, sizeof(pixels));
	memcpy(&pixels[cursor], &colors[color], sizeof(struct led_rgb));
	rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
	if (rc) {
		LOG_ERR("couldn't update strip: %d", rc);
	}
	color = (color + 1) % ARRAY_SIZE(colors);
}

void joystick_down_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick down callback");
}

void joystick_left_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick left callback");
}

void joystick_right_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("joystick right callback");
}

void joystick_center_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
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

/* just for tests: read in border cell map function */
static int read_map(void) {
	int rc;
	size_t maps_len = sizeof(maps) / sizeof(maps[0]);
	int i;
	int index;
	for (i=0; i <= maps_len-1; i++) {
		index = map_pos_to_index(maps[i].position.x,maps[i].position.y);
		memcpy(&pixels[index], &maps[i].color, sizeof(struct led_rgb));
	}
	rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
	if (rc) {
		LOG_ERR("couldn't update strip: %d", rc);
	}
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
			 false)) {
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

	rc = read_map();


	return 0;
}
