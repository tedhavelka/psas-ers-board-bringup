/**
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief ERS "keeper" module, acts like a bulletin board to hold shared data
 *   across the app.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(keeper, LOG_LEVEL_INF);

/*
------------------------------------------------------------------------
Summary of known ERS inputs 2025-09-25, subject to be updated:

Digital:

[x] !UMB_ON (PA8 = GPIO input)
[x] ISO_DROGUE (PA5 = GPIO input)      . . . "Sender" ERS board only
[x] ISO_MAIN  (PA6 = GPIO input)       . . . "Sender" ERS board only
[ ] !MOTOR_FAILA (PB7 = GPIO input)

Analog:

[x] BATT_READ (PB0 = analog input ADC_IN8)
[x] MOTOR_ISENSE (PB1 = Analog input ADC_IN9)
[x] HALL1 (PA0 = Analog input ADC_IN0)
[x] HALL2 (PA1 = Analog input ADC_IN1)

Message based inputs:

[ ] CAN module   . . . given ERS board/firmware variant will keep track
                       of one or more CAN messages, through state
                       variables.
------------------------------------------------------------------------
*/ 

// GPIO inputs, effectively Boolean
static atomic_t iso_drogue = ATOMIC_INIT(0);
static atomic_t iso_main = ATOMIC_INIT(0);
static atomic_t not_umb_on = ATOMIC_INIT(0);
static atomic_t not_motor_faila = ATOMIC_INIT(0);

// analog inputs, typically 12-bit or 16-bit values
static atomic_t batt_read = ATOMIC_INIT(0);
static atomic_t motor_isense = ATOMIC_INIT(0);
static atomic_t hall_1 = ATOMIC_INIT(0);
static atomic_t hall_2 = ATOMIC_INIT(0);

static atomic_t batt_read_dv = ATOMIC_INIT(0);

// Off-chip peripherals and system statae
//
// [ ] ring status    . . . one of 2 = locked, 1 = between, 0 = unlocked
// [x] batt_voltage   . . . among analog inputs
// [ ] batt_ok        . . . a threshold based state
// [ ] shore_power_ok . . . 1 = shore power detected, 0 = no shore power
// [ ] can_bus_ok     . . . telemetrum heartbeat received within last two seconds
// [ ] ready_state    . . . true when (1) battery ok (2) ring locked (3) CAN bus ok
// [ ] reserved
// [ ] reserved

static atomic_t ring_status = ATOMIC_INIT(0);
static atomic_t batt_ok = ATOMIC_INIT(0);
static atomic_t shore_power_ok = ATOMIC_INIT(0);
static atomic_t can_bus_ok = ATOMIC_INIT(0);
static atomic_t ready_state = ATOMIC_INIT(0);

// Support run time toggling of diagnostics which share UART with Zephyr shell:
static atomic_t ers_diag_flag_fs = ATOMIC_INIT(0);

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

// GPIO type inputs
// "set" APIs

void ekset_iso_drogue(const uint32_t value)
{
	atomic_set(&iso_drogue, (atomic_val_t)value);
}

void ekset_iso_main(const uint32_t value)
{
	atomic_set(&iso_main, (atomic_val_t)value);
}

void ekset_not_umb_on(const uint32_t value)
{
	atomic_set(&not_umb_on, (atomic_val_t)value);
}

void ekset_not_motor_faila(const uint32_t value)
{
	atomic_set(&not_umb_on, (atomic_val_t)value);
}

// "get" APIs

void ekget_iso_drogue(uint32_t* value)
{
	*value = atomic_get(&iso_drogue);
}

void ekget_iso_main(uint32_t* value)
{
	*value = atomic_get(&iso_main);
}

void ekget_not_unb_on(uint32_t* value)
{
	*value = atomic_get(&not_umb_on);
}

void ekget_not_motor_faila(uint32_t* value)
{
	*value = atomic_get(&not_motor_faila);
}

//----------------------------------------------------------------------
// - SECTION - ERS analog inputs
//----------------------------------------------------------------------

// "set" APIs

void ekset_batt_read(const uint32_t value)
{
	atomic_set(&batt_read, (atomic_val_t)value);
}

/**
 * @brief store batter voltage converted to decivolts:
 */

void ekset_batt_read_dv(const uint32_t value)
{
	atomic_set(&batt_read_dv, (atomic_val_t)value);
}

void ekset_motor_isense(const uint32_t value)
{
	atomic_set(&motor_isense, (atomic_val_t)value);
}

void ekset_hall_1(const uint32_t value)
{
	atomic_set(&hall_1, (atomic_val_t)value);
}

void ekset_hall_2(const uint32_t value)
{
	atomic_set(&hall_2, (atomic_val_t)value);
}

// "get" APIs

void ekget_batt_read(uint32_t* value)
{
	*value = atomic_get(&batt_read);
}

void ekget_batt_read_dv(uint32_t* value)
{
	*value = atomic_get(&batt_read_dv);
}

void ekget_motor_isense(uint32_t* value)
{
	*value = atomic_get(&motor_isense);
}

void ekget_hall_1(uint32_t* value)
{
	*value = atomic_get(&hall_1);
}

void ekget_hall_2(uint32_t* value)
{
	*value = atomic_get(&hall_2);
}

//----------------------------------------------------------------------
// - SECTION - Off-chip peripherals and system statae
//----------------------------------------------------------------------

// "set" APIs

void ekset_ring_status(const uint32_t value)
{
	atomic_set(&ring_status, (atomic_val_t)value);
}

void ekset_batt_ok(const uint32_t value)
{
	atomic_set(&batt_ok, (atomic_val_t)value);
}

void ekset_shore_power_ok(const uint32_t value)
{
	atomic_set(&shore_power_ok, (atomic_val_t)value);
}

void ekset_can_bus_ok(const uint32_t value)
{
	atomic_set(&can_bus_ok, (atomic_val_t)value);
}

void ekset_ready_state(const uint32_t value)
{
	atomic_set(&ready_state, (atomic_val_t)value);
}

// "get" APIs

void ekget_ring_status(uint32_t* value)
{
	*value = atomic_get(&ring_status);
}

void ekget_batt_ok(uint32_t* value)
{
	*value = atomic_get(&batt_ok);
}

void ekget_shore_power_ok(uint32_t* value)
{
	*value = atomic_get(&shore_power_ok);
}

void ekget_can_bus_ok(uint32_t* value)
{
	*value = atomic_get(&can_bus_ok);
}

void ekget_ready_state(uint32_t* value)
{
	*value = atomic_get(&ready_state);
}

//----------------------------------------------------------------------
// - SECTION - ERS diagnostics
//----------------------------------------------------------------------

/**
 * @brief ERS firmware sends periodic diagnostic and state info over the debug
 *   UART.  This pair of routines enables and disables this at run time.
 */

void ek_sys_diag_periodic(void)
{
	atomic_set(&ers_diag_flag_fs, (atomic_val_t)true);
}

void ek_sys_diag_quiet(void)
{
	atomic_set(&ers_diag_flag_fs, (atomic_val_t)false);
}

void ek_get_sys_diag_mode(uint32_t* value)
{
	*value = atomic_get(&ers_diag_flag_fs);
}
