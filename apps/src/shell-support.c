/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief ERS shell support module provides a dedicated thread from which to
 *   start Zephyr shell.
 *
 * @note Macros to set up Zephyr shell expand to code which does not appear to
 *   return.  This module implementing a thread permits the application's
 *   default thread "main" to execute beyond the calls to set up and begin
 *   executing Zephyr shell facility.
 */

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(shell_support, LOG_LEVEL_INF);

#include <arbiter.h>
#include <ers-adc.h>
#include <keeper.h>

#define SHELL_SUPPORT_THREAD_STACK_SIZE 512
#define SHELL_SUPPORT_THREAD_PRIORITY 5

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#if 0
K_THREAD_STACK_DEFINE(shell_support_thread_stack, SHELL_SUPPORT_THREAD_STACK_SIZE);

struct k_thread shell_support_thread_data;
#endif

static const struct shell *shell_ptr_fs = NULL;

static uint32_t dev_test_calls_fs = 0;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

/**
 * @brief ERS app incorporates Zephyr shell and adds custom commands to
 *   this facility.
 *
 * @note
 */

static int ers_cmd_wrapper_read_adc_all(const struct shell *shell, size_t argc, char *argv[])
{
	int rc = cmd_ers_read_adc_all(shell);
	return rc;
}

static int ers_cmd_wrapper_read_adc_in0(const struct shell *shell, size_t argc, char *argv[])
{
	int rc = cmd_ers_read_adc_in0(shell);
	return rc;
}

static int ers_cmd_print_mark(const struct shell *shell, size_t argc, char *argv[])
{
	shell_fprintf(shell, SHELL_NORMAL, "- MARK -\n");
        shell_ptr_fs = shell;
	return 0;
}

static int ers_cmd_print_shell_addr(const struct shell *shell, size_t argc, char *argv[])
{
	shell_fprintf(shell, SHELL_NORMAL, "Current shell at addr 0x%" PRIxPTR "\n",
		      (long unsigned int)shell_ptr_fs);
	shell_fprintf(shell, SHELL_NORMAL, "DEV 0926 app shell print test called %u "
		      " times\n", dev_test_calls_fs);
	return 0;
}

/**
 * @brief A development test looking at how and whether possible to get
 *   reference to Zephyr shell structure at run time.
 */

int32_t dev_test_of_shell_printing_from_app(const char* message)
{
	int32_t rc = 0;
	dev_test_calls_fs++;

	if (shell_ptr_fs == NULL)
	{
		return -EINVAL;
	}

	shell_fprintf(shell_ptr_fs, SHELL_NORMAL, "%s", message);
	shell_fprintf(shell_ptr_fs, SHELL_NORMAL, "- M1 -\n");
	return rc;
}

static int ers_cmd_diag_periodic_on(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	ek_sys_diag_periodic();
	return 0;
}

static int ers_cmd_diag_periodic_off(const struct shell *shell, size_t argc, char *argv[])
{
        ARG_UNUSED(shell);
        ARG_UNUSED(argc);
        ARG_UNUSED(argv);

	ek_sys_diag_quiet();
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	ers_cmds,
	SHELL_CMD_ARG(adcall, NULL,
		"Read ERS board's four ADC channels",
		ers_cmd_wrapper_read_adc_all, 0, 0),
	SHELL_CMD_ARG(adc0, NULL,
		"Read ERS board ADC for Hall sensor 1",
		ers_cmd_wrapper_read_adc_in0, 0, 0),
#if 0
	SHELL_CMD_ARG(adc1, NULL,
		"Read ERS board ADC for Hall sensor 2",
		cmd_ers_read_adc_in1, 0, 0),
#endif
	SHELL_CMD_ARG(print_mark, NULL,
		"output to shell console a brief 'mark' message",
		ers_cmd_print_mark, 0, 0),
	SHELL_CMD_ARG(print_shell_ptr, NULL,
		"print address of run time Zephyr shell instance",
		ers_cmd_print_shell_addr, 0, 0),
	SHELL_SUBCMD_SET_END
	);

SHELL_CMD_REGISTER(ers, &ers_cmds, "ERS commands", NULL);

SHELL_STATIC_SUBCMD_SET_CREATE(
	ers_cmds_diag,
	SHELL_CMD_ARG(on, NULL,
		"enable ERS periodic diagnostics",
		ers_cmd_diag_periodic_on, 0, 0),
	SHELL_CMD_ARG(off, NULL,
		"disable ERS periodic diagnostics",
		ers_cmd_diag_periodic_off, 0, 0),
	SHELL_SUBCMD_SET_END
	);

SHELL_CMD_REGISTER(diag, &ers_cmds_diag, "ERS diagnostics", NULL);

// - DEV 1001 BEGIN -
#if 0
	SHELL_STATIC_SUBCMD_SET_CREATE(
		ers_cmds_hall,
		SHELL_CMD_ARG(, NULL,
#endif

static int cmd1_handler(const struct shell *sh, size_t argc, char **argv)
{
        shell_print(sh, "cmd1 executed");
        return 0;
}

static int cmd2_handler(const struct shell *sh, size_t argc, char **argv)
{
        shell_print(sh, "cmd2 executed");
        return 0;
}

// (1)
SHELL_SUBCMD_SET_CREATE(sub_section_cmd, (section_cmd));

// (2)
/* Create a set of subcommands for "section_cmd cm1". */
// SHELL_SUBCMD_SET_CREATE(sub_section_cmd1, (section_cmd, cmd1));
SHELL_SUBCMD_SET_CREATE(sub_section_cmd1, (section_cmd, cmd1, cmd2));

// (3)
/* Add command to the set. Subcommand set is identify by parent shell command. */
SHELL_SUBCMD_ADD((section_cmd), cmd1, &sub_section_cmd1, "help for cmd1", cmd1_handler, 1, 0); 

SHELL_SUBCMD_ADD((section_cmd), cmd2, &sub_section_cmd1, "help for cmd2", cmd2_handler, 1, 0); 

// (4)
SHELL_CMD_REGISTER(section_cmd, &sub_section_cmd,
                   "Demo command using section for subcommand registration", NULL);



// Hall set cutoff command

SHELL_SUBCMD_SET_CREATE(sub_section_hall, (hall));

/* Create a set of one subcommand for 'hall' command */
SHELL_SUBCMD_SET_CREATE(sub_section_hall_set, (hall, set));

SHELL_SUBCMD_ADD((hall), v_under_cutoff, &sub_section_hall_set, "set Hall state voltage under cutoff", shell_wrapper_set_v_under_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), inactive_cutoff, &sub_section_hall_set, "set Hall state inactive cutoff", shell_wrapper_set_inactive_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), between_cutoff, &sub_section_hall_set, "set Hall state between cutoff", shell_wrapper_set_between_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), active_cutoff, &sub_section_hall_set, "set Hall state active cutoff", shell_wrapper_set_active_cutoff, 2, 0);

SHELL_SUBCMD_ADD((hall), show_cutoffs, &sub_section_hall, "show Hall state cutoff values", arbiter_show_hall_state_cutoffs, 1, 0);

SHELL_CMD_REGISTER(hall, &sub_section_hall, "set and show Hall state cutoff values (in ADC counts)", NULL);

#if 0
SHELL_STATIC_SUBCMD_SET_CREATE(
	hall_show_cmds,
	SHELL_CMD_ARG(show_cutoffs, NULL,
		"show Hall sensor state cutuffs (in ADC counts)",
		arbiter_show_hall_state_cutoffs, 0, 0),
	SHELL_SUBCMD_SET_END
	);
#endif

// SHELL_CMD_REGISTER(hall, &hall_show_cmds, "show Hall state cutoff values", NULL);

// - DEV 1001 END -



int32_t ers_init_shell_support(void)
{
	int32_t rc = 0;
	return rc;
}
