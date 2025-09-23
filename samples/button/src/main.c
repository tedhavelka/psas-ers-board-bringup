/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

// #define SLEEP_TIME_MS	1
#define SLEEP_TIME_MS	1000

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});

// - DEV 0918 BEGIN -
//  Add a second button, questions yet about how and whether to
//  create a second instance of "button_cb_data" a few lines ahead:
#define SW1_NODE	DT_ALIAS(sw1)
#if !DT_NODE_HAS_STATUS(SW1_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios,
							      {0});
// - DEV 0918 END -

static struct gpio_callback button_cb_data;

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

int32_t configure_button_iso_main(void)
{
	int32_t rc = 0;

	if (!gpio_is_ready_dt(&button_1)) {
		printk("Error: button_1 device %s is not ready\n",
		       button_1.port->name);
		return -EIO;
	}

	rc = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (rc != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       rc, button_1.port->name, button_1.pin);
		return -EINVAL;
	}

	rc = gpio_pin_interrupt_configure_dt(&button_1,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (rc != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			rc, button_1.port->name, button_1.pin);
		return -EINVAL;
	}

	return 0;
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return 0;
	}

	ret = configure_button_iso_main();
        printk("To configure second button returns %d", ret);

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	printk("Set up button at %s pin %d\n", button.port->name, button.pin);

	if (led.port && !gpio_is_ready_dt(&led)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led.port->name);
		led.port = NULL;
	}
	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}

	if (led.port) {
		printk("port for LED ok\n");
	}
	else
	{
		printk("port for LED not configured!\n");
	}

	printk("Press the button\n");
	// if (led.port) {
		while (1) {
			/* If we have an LED, match its state to the button's. */
			int val = gpio_pin_get_dt(&button);
			int val_iso_main = gpio_pin_get_dt(&button_1);

			if (val >= 0) {
				gpio_pin_set_dt(&led, val);
			}
			else
			{
				printk("Latest poll of button state returns %d\n", val);
			}

			printk("ISO_MAIN pin read returns value of %d\n", val_iso_main);

			k_msleep(SLEEP_TIME_MS);
		}
	// }
	return 0;
}
