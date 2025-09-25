# Portland State Aerospace Society (c) 2025
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# execute_process(
#     COMMAND git rev-parse --show-toplevel
#     OUTPUT_VARIABLE ERS_REPO_ROOT
#     OUTPUT_STRIP_TRAILING_WHITESPACE
#     COMMAND_ERROR_IS_FATAL ANY
# )

set(CONF_FILE prj.conf ers-log-levels.conf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ers_drogue_board)

target_include_directories(app PRIVATE
    include
    # ${ERS_REPO_ROOT}/shell-module/include
    shell-module/include
)

target_sources(app PRIVATE
    src/adc.c
    src/can.c
    src/main.c
)

# Zephyr shell addition to ERS board firmware:

target_sources(app PRIVATE
    shell-module/src/shell-module.c
    shell-module/src/test_module.c
)

target_sources_ifdef(CONFIG_SHELL_DYNAMIC_CMDS app PRIVATE
    shell-module/src/dynamic_cmd.c
)

target_sources_ifdef(CONFIG_SHELL_BACKEND_SERIAL app PRIVATE
    shell-module/src/uart_reinit.c
)
