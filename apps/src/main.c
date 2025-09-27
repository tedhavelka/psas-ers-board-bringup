/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <ers-adc.h>
#include <ers-can.h>
#include <gpio-in.h>
#include <keeper.h>
#include <shell-support.h>

LOG_MODULE_REGISTER(ers_main, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ERS_MAIN_LOOP_PERIOD_MS 5000

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int main(void)
{
	static uint32_t loop_count = 0;
	int32_t rc = 0;

	rc = adc_init();
	LOG_INF("adc_init() returns %d", rc);

// TODO [ ] Create thread for CAN init routine which entails a `while (1)`
//  construct.
	rc = ers_init_can();
	LOG_INF("ERS CAN module init returns %d", rc);

        rc = ers_init_shell_support();
	LOG_INF("ERS command initialization returns %d", rc);

        rc = ers_init_gpio_in();
	LOG_INF("GPIO input pin initialization returns %d", rc);

	LOG_INF("main() entering 'while (1)' loop . . .");

	while (1)
	{
		loop_count++;
		ek_get_sys_diag_mode(&rc);
		if (rc > 0)
		{
			LOG_INF("- MARK -");
		}
		k_msleep(ERS_MAIN_LOOP_PERIOD_MS);
	}

	return 0;
}
