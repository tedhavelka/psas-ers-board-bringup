/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(arbiter, LOG_LEVEL_INF);

#include <ers-can.h>
#include <ers-dac.h>
#include <gpio-in.h>
#include <keeper.h>

#define ERS_ARBITER_SLEEP_PER_MS 2000

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#define ARBITER_THREAD_STACK_SIZE 512
#define ARBITER_THREAD_PRIORITY 1
K_THREAD_STACK_DEFINE(arbiter_thread_stack, ARBITER_THREAD_STACK_SIZE);
struct k_thread arbiter_thread_data;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

void arbiter_thread_entry(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

	static uint32_t loop_count = 0;
	int32_t rc = 0;

	while (1)
	{
		// LOG_INF("M3");
		LOG_INF("setting deploy1 GPIO to %d", (loop_count % 2));
#if 1
		rc = ers_gpios_set_deploy1(loop_count % 2);
		LOG_INF("GPIO set returns status %d", rc);
		rc = ers_gpios_set_deploy2((loop_count + 1) % 2);
		LOG_INF("GPIO set returns status %d", rc);
#endif
		loop_count++;
		k_msleep(ERS_ARBITER_SLEEP_PER_MS);
	}
}

int32_t ers_init_arbiter(void)
{
	int32_t rc = 0;

	k_tid_t arbiter_tid = k_thread_create(&arbiter_thread_data, arbiter_thread_stack,
					 K_THREAD_STACK_SIZEOF(arbiter_thread_stack),
					 arbiter_thread_entry, NULL, NULL, NULL,
				 ARBITER_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!arbiter_tid) {
		LOG_ERR("ERROR spawning rx thread\n");
	}

	return rc;
}
