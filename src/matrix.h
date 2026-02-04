#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>

#if DT_NODE_HAS_PROP(DT_ALIAS(led_strip), chain_length)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)
#else
#error Unable to determine length of LED strip
#endif

int init_matrix(void);
int display (struct led_rgb * pixels);