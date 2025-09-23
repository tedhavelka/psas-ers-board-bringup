/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <ers-adc.h>

LOG_MODULE_REGISTER(ers_main, LOG_LEVEL_INF);

int main(void)
{
	static uint32_t loop_count = 0;
	int32_t rc = 0;

	rc = adc_init();
	LOG_INF("adc_init() returns %d", rc);        

	while (1)
	{
		loop_count++;
		// printk("Hello World! %s iter %u\n", CONFIG_BOARD_TARGET, loop_count);
		LOG_INF("Hello World! %s iter %u\n", CONFIG_BOARD_TARGET, loop_count);
		k_msleep(1000);
	}

	return 0;
}
