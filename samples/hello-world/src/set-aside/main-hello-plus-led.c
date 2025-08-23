/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#if !DT_NODE_EXISTS(DT_NODELABEL(led_green))
#error "Overlay for !umb_on node not properly defined."
#endif

static const struct gpio_dt_spec led_green =
	GPIO_DT_SPEC_GET_OR(DT_NODELABEL(led_green), gpios, {0});

int main(void)
{
	static uint32_t loop_count = 0;
	bool toggle_led_ready = false;
	int32_t rc = 0;

	if (!gpio_is_ready_dt(&led_green))
	{
		printf("NOT_UMB_ON signal out GPIO is not ready.\n");
		// return 0;
	}
	else
	{
		toggle_led_ready = true;
	}

	if (toggle_led_ready)
	{ 
		printk("main.c configurinng GPIO for led_green . . .");
		rc = gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
	}

	if (rc)
	{
		printk("Failed to configure GPIO for led_green, err %d", rc);
		toggle_led_ready = false;
	}

	while (1)
	{
		loop_count++;
		printk("Hello World! %s\n", CONFIG_BOARD_TARGET);
		k_msleep(1000);

		if (toggle_led_ready)
		{
			if (loop_count % 2)
			{
				rc = gpio_pin_set_dt(&led_green, 1);
			}
			else
			{
				rc = gpio_pin_set_dt(&led_green, 0);
			}

			if (rc)
			{
				printk("Failed to set GPIO, err %d", rc);
			}
		}
	}

	return 0;
}
