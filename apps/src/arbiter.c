/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(arbiter, LOG_LEVEL_INF);

#include <ers-can.h>
#include <ers-dac.h>
#include <gpio-in.h>
#include <keeper.h>

//----------------------------------------------------------------------
// - SECTION - defines
//----------------------------------------------------------------------

/*
        Hall2 |
 Hall1        |   Over     Active    Between   Inactive    Under
--------------+------------------------------------------------------
Over          |   error    locked    betwen    unlocked    error
Active        |  unlocked   error    between   UNLOCKED   unlocked
Between       |   between  between   BETWEEN    between    between
Unactive      |   locked    LOCKED   between     error     locked
Under         |   error     locked   between   unlocked     error

ADC channels are 12-bit, hence ADC counts range 0..4095.  Define
"Under" through "Over" subranges:
*/

// clang-format off
// Note these values from Hessah of PSAS, cerca 2025-09-25.  Set of like data
// points from a second tested ERS board differ.
#define HALL_READING_V_UNDER_CUTOFF   600
#define HALL_READING_INACTIVE_CUTOFF  700
#define HALL_READING_BETWEEN_CUTOFF  1600
#define HALL_READING_ACTIVE_CUTOFF   3100
// ADC Hall sensor readings above HALL_READING_ACTIVE_CUTOFF considered "Over".
// clang-format on

#define BASE_10 10

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#define ERS_ARBITER_SLEEP_PER_MS 2000

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#define ARBITER_THREAD_STACK_SIZE 512
#define ARBITER_THREAD_PRIORITY 1
K_THREAD_STACK_DEFINE(arbiter_thread_stack, ARBITER_THREAD_STACK_SIZE);
struct k_thread arbiter_thread_data;

enum hall_sensor_state {
	HALL_OUTPUT_UNDER_VOLTAGE,
	HALL_OUTPUT_INACTIVE,
	HALL_OUTPUT_BETWEEN,
	HALL_OUTPUT_ACTIVE,
	HALL_OUTPUT_OVER_VOLTAGE
};

static atomic_t hall_reading_v_under_cutoff_fs = ATOMIC_INIT(HALL_READING_V_UNDER_CUTOFF);
static atomic_t hall_reading_inactive_cutoff_fs = ATOMIC_INIT(HALL_READING_INACTIVE_CUTOFF);
static atomic_t hall_reading_between_cutoff_fs = ATOMIC_INIT(HALL_READING_BETWEEN_CUTOFF);
static atomic_t hall_reading_active_cutoff_fs = ATOMIC_INIT(HALL_READING_ACTIVE_CUTOFF);

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

// Routines to accept and store Hall sensor threasholds, such as under voltage.

// Setter functions for hall state cutuffs:

void arbiter_set_v_under_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_v_under_cutoff_fs, (atomic_val_t)value);
}

void arbiter_set_inactive_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_inactive_cutoff_fs, (atomic_val_t)value);
}

void arbiter_set_between_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_between_cutoff_fs, (atomic_val_t)value);
}

void arbiter_set_active_cutoff(const uint32_t value)
{
	atomic_set(&hall_reading_active_cutoff_fs, (atomic_val_t)value);
}

// TODO [ ] Add check of 'endptr' to determine whether we got valid numeric input,
//  in all routines which call strtol():

void shell_wrapper_set_v_under_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	shell_fprintf(shell, SHELL_NORMAL, "from user got v_under_cutoff of %u\n",
		      value);
	arbiter_set_v_under_cutoff(value);
}

void shell_wrapper_set_inactive_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	arbiter_set_inactive_cutoff(value);
}

void shell_wrapper_set_between_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	arbiter_set_between_cutoff(value);
}

void shell_wrapper_set_active_cutoff(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t value = 0;
	char *endptr, *str;
	str = argv[1];
	value = strtol(str, &endptr, BASE_10);
	arbiter_set_active_cutoff(value);
}

// Getter functions for hall state cutuffs:

void arbiter_get_v_under_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_v_under_cutoff_fs);
}

void arbiter_get_inactive_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_inactive_cutoff_fs);
}

void arbiter_get_between_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_between_cutoff_fs);
}

void arbiter_get_active_cutoff(uint32_t *value)
{
	*value = atomic_get(&hall_reading_active_cutoff_fs);
}

/**
 * @brief Routine to report Hall sensor state cutoff values (in ADC counts).
 */

void arbiter_show_hall_state_cutoffs(const struct shell *shell)
{
	uint32_t v_under_cutoff, inactive_cutoff, between_cutoff, active_cutoff;

	arbiter_get_v_under_cutoff(&v_under_cutoff);
	arbiter_get_inactive_cutoff(&inactive_cutoff);
	arbiter_get_between_cutoff(&between_cutoff);
	arbiter_get_active_cutoff(&active_cutoff);

	shell_fprintf(shell, SHELL_NORMAL, "Hall sensor state cutoff values (in ADC "
	  "counts):\n");
	shell_fprintf(shell, SHELL_NORMAL, "  v under cutoff: %u\n", v_under_cutoff);
	shell_fprintf(shell, SHELL_NORMAL, " inactive cutoff: %u\n", inactive_cutoff);
	shell_fprintf(shell, SHELL_NORMAL, "  between cutoff: %u\n", between_cutoff);
	shell_fprintf(shell, SHELL_NORMAL, "   active cutoff: %u\n", active_cutoff);
}

/**
 * @brief Routine to categorize ADC readings of lock ring Hall sensor into one
 *   of five states.
 */

enum hall_sensor_state arbiter_adc_reading_to_hall_state(const uint32_t adc_reading)
{
	enum hall_sensor_state sensor_state = HALL_OUTPUT_OVER_VOLTAGE;

	if (adc_reading < HALL_READING_V_UNDER_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_UNDER_VOLTAGE;
	}

	if (adc_reading < HALL_READING_INACTIVE_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_INACTIVE;
	}

	if (adc_reading < HALL_READING_BETWEEN_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_BETWEEN;
	}

	if (adc_reading < HALL_READING_ACTIVE_CUTOFF)
	{
		sensor_state = HALL_OUTPUT_ACTIVE;
	}

	return sensor_state;
}

void arbiter_thread_entry(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

	static uint32_t loop_count = 0;
	// int32_t rc = 0;

	while (1)
	{
		// LOG_INF("M3");
#if 0
		LOG_INF("setting deploy1 GPIO to %d", (loop_count % 2));
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

	// Initialize Hall sensor ADC count threshold values:
	// (These values used to determine practical Hall sensor states)

	arbiter_set_v_under_cutoff(HALL_READING_V_UNDER_CUTOFF);
	arbiter_set_inactive_cutoff(HALL_READING_INACTIVE_CUTOFF);
	arbiter_set_between_cutoff(HALL_READING_BETWEEN_CUTOFF);
	arbiter_set_active_cutoff(HALL_READING_ACTIVE_CUTOFF);

	k_tid_t arbiter_tid = k_thread_create(&arbiter_thread_data, arbiter_thread_stack,
					 K_THREAD_STACK_SIZEOF(arbiter_thread_stack),
					 arbiter_thread_entry, NULL, NULL, NULL,
				 ARBITER_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!arbiter_tid) {
		LOG_ERR("ERROR spawning rx thread\n");
	}

	return rc;
}
