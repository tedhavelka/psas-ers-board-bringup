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
// Note ERS shell module, based on Zephyr shell sample app, has its sources
//  organized in a separate source directory alongside 'src', but ERS shell
//  module header files are place in the 'include' dir:
#include <shell-module.h>
#include <shell-support.h>

LOG_MODULE_REGISTER(ers_main, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ERS_MAIN_LOOP_PERIOD_MS 5000

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

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
	// rc = ers_init_can();
	// LOG_INF("ERS CAN module init returns %d", rc);

// TODO [ ] Check whether shell-module sources are needed.
	// rc = shell_module_init();
	// LOG_INF("Zephyr shell module init returns %d", rc);

	LOG_INF("Initializing ERS specific commands . . .");
        rc = ers_init_shell_support();
	LOG_INF("M2");

	while (1)
	{
		loop_count++;
		LOG_INF("- MARK -");

		rc = dev_test_of_shell_printing_from_app("- DEV 0926 - printing "
                  "from Zephyr shell programmatically!\n");
		if (rc != 0)
                { }

		k_msleep(ERS_MAIN_LOOP_PERIOD_MS);
	}

	return 0;
}
