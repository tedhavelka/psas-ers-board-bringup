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

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(shell_support, LOG_LEVEL_INF);

#include <ers-adc.h>
#include <keeper.h>

#define SHELL_SUPPORT_THREAD_STACK_SIZE 512
#define SHELL_SUPPORT_THREAD_PRIORITY 5

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

K_THREAD_STACK_DEFINE(shell_support_thread_stack, SHELL_SUPPORT_THREAD_STACK_SIZE);

struct k_thread shell_support_thread_data;

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

void shell_support_thread(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

	LOG_INF("Shell support thread starting . . .");

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

	// shell_set_root_cmd("ers_commands");

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
}

int32_t ers_init_shell_support(void)
{
	int32_t rc = 0;

        k_tid_t shell_support_tid = k_thread_create(&shell_support_thread_data,
				 shell_support_thread_stack,
                                 K_THREAD_STACK_SIZEOF(shell_support_thread_stack),
                                 shell_support_thread, NULL, NULL, NULL,
                                 SHELL_SUPPORT_THREAD_PRIORITY, 0, K_NO_WAIT);
        if (!shell_support_tid) {
                LOG_ERR("ERROR spawning shell support thread\n");
        }

	return rc;
}
