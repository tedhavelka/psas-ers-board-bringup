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
[?] MOTOR_ISENSE (PB1 = Analog input ADC_IN9)
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

// GPIO type inputs

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
