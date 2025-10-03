#ifndef ERS_ARBITER_H
#define ERS_ARBITER_H

#include <zephyr/shell/shell.h>

int32_t ers_init_arbiter(void);

void arbiter_set_v_under_cutoff(const uint32_t value);
void arbiter_set_inactive_cutoff(const uint32_t value);
void arbiter_set_between_cutoff(const uint32_t value);
void arbiter_set_active_cutoff(const uint32_t value);

void arbiter_get_v_under_cutoff(uint32_t *value);
void arbiter_get_inactive_cutoff(uint32_t *value);
void arbiter_get_between_cutoff(uint32_t *value);
void arbiter_get_active_cutoff(uint32_t *value);

void shell_wrapper_set_v_under_cutoff(const struct shell *shell, size_t argc, char **argv);
void shell_wrapper_set_inactive_cutoff(const struct shell *shell, size_t argc, char **argv);
void shell_wrapper_set_between_cutoff(const struct shell *shell, size_t argc, char **argv);
void shell_wrapper_set_active_cutoff(const struct shell *shell, size_t argc, char **argv);

void arbiter_show_hall_state_cutoffs(const struct shell *shell);

#endif // ERS_ARBITER_H
