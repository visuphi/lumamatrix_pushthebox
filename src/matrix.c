#include <matrix.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(matrix);

#define STRIP_NODE DT_ALIAS(led_strip)

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

int init_matrix (void) {
	if ( device_is_ready(strip) ) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return -1;
	}
	return 0;
}

int display (struct led_rgb * pixels) {
	int rc;
	rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
	if (rc) {
		LOG_ERR("couldn't update strip: %d", rc);
		return rc;
	}
    return 0;
}