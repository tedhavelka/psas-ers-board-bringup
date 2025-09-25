/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <shell/shell.h>

#include <ers-adc.h>
#include <ers-can.h>
#include <shell-module.h>

LOG_MODULE_REGISTER(ers_main, LOG_LEVEL_INF);

#define ERS_MAIN_LOOP_PERIOD_MS 10000

int main(void)
{
	static uint32_t loop_count = 0;
	int32_t rc = 0;

	rc = adc_init();
	LOG_INF("adc_init() returns %d", rc);

	rc = ers_can_init();
	LOG_INF("ERS CAN module init returns %d", rc);

	rc = shell_module_init();
	LOG_INF("Zephyr shell module init returns %d", rc);

	while (1)
	{
		loop_count++;
		// LOG_INF("Hello World! %s iter %u\n", CONFIG_BOARD_TARGET, loop_count);
		LOG_INF("- MARK -");
		k_msleep(ERS_MAIN_LOOP_PERIOD_MS);
	}

	return 0;
}
