#ifndef ERS_KEEPER_H
#define ERS_KEEPER_H

void ekset_iso_drogue(const uint32_t value);
void ekset_iso_main(const uint32_t value);
void ekset_not_umb_on(const uint32_t value);
void ekset_not_motor_faila(const uint32_t value);

void ekget_iso_drogue(uint32_t* value);
void ekget_iso_main(uint32_t* value);
void ekget_not_umb_on(uint32_t* value);
void ekget_not_motor_faila(uint32_t* value);

void ek_sys_diag_periodic(void);
void ek_sys_diag_quiet(void);
void ek_get_sys_diag_mode(uint32_t* value);

#endif // ERS_KEEPER_H
