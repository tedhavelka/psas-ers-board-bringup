/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <ers-adc.h>
#include <ers-can.h>
#include <shell-module.h>

LOG_MODULE_REGISTER(ers_main, LOG_LEVEL_INF);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ERS_MAIN_LOOP_PERIOD_MS 10000

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

static const struct shell *shell_ptr_fs = NULL;

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
	shell_fprintf(shell, SHELL_NORMAL, "Current shell at addr 0x%" PRIxPTR "\n", (long unsigned int)shell_ptr_fs);
	return 0;
}

void init_ers_custom_commands(void)
{
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

	SHELL_CMD_REGISTER(ers_commands, &ers_cmds, "ERS commands", NULL);

	shell_set_root_cmd("ers_commands");
}

int main(void)
{
	static uint32_t loop_count = 0;
	int32_t rc = 0;

	rc = adc_init();
	LOG_INF("adc_init() returns %d", rc);

	rc = ers_can_init();
	LOG_INF("ERS CAN module init returns %d", rc);

	rc = shell_module_init();
	LOG_INF("Zephyr shell module init returns %d", rc);

	LOG_INF("Initializing ERS specific commands . . .");
        init_ers_custom_commands();

	while (1)
	{
		loop_count++;
		// LOG_INF("Hello World! %s iter %u\n", CONFIG_BOARD_TARGET, loop_count);
		LOG_INF("- MARK -");
		// shell_fprintf(shell, SHELL_NORMAL, "- MARK -");
		k_msleep(ERS_MAIN_LOOP_PERIOD_MS);
	}

	return 0;
}
