#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(buttons, LOG_LEVEL_INF);

static const struct gpio_dt_spec joystick_up = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec joystick_down = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const struct gpio_dt_spec joystick_left = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const struct gpio_dt_spec joystick_right = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);
static const struct gpio_dt_spec joystick_center = GPIO_DT_SPEC_GET(DT_ALIAS(sw4), gpios);

static struct gpio_callback joystick_up_cb_data;
static struct gpio_callback joystick_down_cb_data;
static struct gpio_callback joystick_left_cb_data;
static struct gpio_callback joystick_right_cb_data;
static struct gpio_callback joystick_center_cb_data;

static int setup_button(const struct gpio_dt_spec *btn, struct gpio_callback *cb,
			gpio_callback_handler_t handler, bool enable_irq)
{
	int rc;

	if (!gpio_is_ready_dt(btn)) {
		printk("Error: button device %s is not ready\n", btn->port->name);
		return -ENODEV;
	}

	rc = gpio_pin_configure_dt(btn, GPIO_INPUT);
	if (rc != 0) {
		printk("Error %d: failed to configure %s pin %d\n", rc, btn->port->name, btn->pin);
		return rc;
	}

	if (enable_irq) {
		rc = gpio_pin_interrupt_configure_dt(btn, GPIO_INT_EDGE_TO_ACTIVE);
		if (rc != 0) {
			printk("Error %d: failed to configure interrupt on %s pin %d\n", rc,
			       btn->port->name, btn->pin);
			return rc;
		}
	}

	gpio_init_callback(cb, handler, BIT(btn->pin));
	gpio_add_callback(btn->port, cb);

	return 0;
}

int init_buttons(gpio_callback_handler_t up_handler, gpio_callback_handler_t down_handler,
		 gpio_callback_handler_t left_handler, gpio_callback_handler_t right_handler,
		 gpio_callback_handler_t center_handler)
{
	int rc;

	rc = setup_button(&joystick_up, &joystick_up_cb_data, up_handler, true);
	if (rc) {
		return rc;
	}

	rc = setup_button(&joystick_down, &joystick_down_cb_data, down_handler, true);
	if (rc) {
		return rc;
	}

	rc = setup_button(&joystick_left, &joystick_left_cb_data, left_handler, true);
	if (rc) {
		return rc;
	}

	rc = setup_button(&joystick_right, &joystick_right_cb_data, right_handler, true);
	if (rc) {
		return rc;
	}

	rc = setup_button(&joystick_center, &joystick_center_cb_data, center_handler, true);
	if (rc) {
		return rc;
	}

	return 0;
}
