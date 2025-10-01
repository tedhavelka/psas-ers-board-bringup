#ifndef ERS_KEEPER_H
#define ERS_KEEPER_H

// Digital inputs

void ekset_iso_drogue(const uint32_t value);
void ekset_iso_main(const uint32_t value);
void ekset_not_umb_on(const uint32_t value);
void ekset_not_motor_faila(const uint32_t value);

void ekget_iso_drogue(uint32_t* value);
void ekget_iso_main(uint32_t* value);
void ekget_not_umb_on(uint32_t* value);
void ekget_not_motor_faila(uint32_t* value);

// Analog inputs

void ekset_batt_read(const uint32_t value);
void ekset_batt_read_dv(const uint32_t value);
void ekset_motor_isense(const uint32_t value);
void ekset_hall_1(const uint32_t value);
void ekset_hall_2(const uint32_t value);

void ekget_batt_read(uint32_t* value);
void ekget_batt_read_dv(uint32_t* value);
void ekget_motor_isense(uint32_t* value);
void ekget_hall_1(uint32_t* value);
void ekget_hall_2(uint32_t* value);

// Off-chip peripherals and system statae

void ekset_ring_status(const uint32_t value);
void ekset_batt_ok(const uint32_t value);
void ekset_shore_power_ok(const uint32_t value);
void ekset_can_bus_ok(const uint32_t value);
void ekset_ready_state(const uint32_t value);

void ekget_ring_status(uint32_t* value);
void ekget_batt_ok(uint32_t* value);
void ekget_shore_power_ok(uint32_t* value);
void ekget_can_bus_ok(uint32_t* value);
void ekget_ready_state(uint32_t* value);

// ERS diagnostics

void ek_sys_diag_periodic(void);
void ek_sys_diag_quiet(void);
void ek_get_sys_diag_mode(uint32_t* value);

#endif // ERS_KEEPER_H
