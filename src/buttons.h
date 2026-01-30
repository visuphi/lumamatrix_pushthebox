#include <zephyr/drivers/gpio.h>

int init_buttons(gpio_callback_handler_t up_handler, gpio_callback_handler_t down_handler,
		 gpio_callback_handler_t left_handler, gpio_callback_handler_t right_handler,
		 gpio_callback_handler_t center_handler);
