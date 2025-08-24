#!/bin/bash

# /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/bin/openocd -f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/interface/stlink.cfg -f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/target/stm32f0x.cfg -s /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts  '-c init' '-c targets' -c 'reset init' -c 'flash write_image erase /home/ted/projects/zephyr-project/psas-ers-firmware/samples/hello-world/build/zephyr/zephyr.hex' -c 'reset run' -c shutdown

# In a gdb client session:
#
# (gdb) target extended-remote localhost:3333

function flash_only()
{
	/opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/bin/openocd \
	-f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/interface/stlink.cfg \
	-f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/target/stm32f0x.cfg \
	-s /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts \
	'-c init' \
	'-c targets' \
	-c 'reset init' \
	-c 'flash write_image erase /home/ted/projects/zephyr-project/psas-ers-firmware/samples/hello-world/build/zephyr/zephyr.hex' \
	-c 'reset run' \
	-c shutdown
}

function flash_and_enter_debugger()
{
	/opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/bin/openocd \
	-f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/interface/stlink.cfg \
	-f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/target/stm32f0x.cfg \
	-s /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts \
	'-c init' \
	'-c targets' \
	-c 'reset init' \
	-c 'flash write_image erase /home/ted/projects/zephyr-project/psas-ers-firmware/samples/hello-world/build/zephyr/zephyr.hex' \
	-c 'reset run'
}

function flash_with_psas_recovery_board_options()
{
	OPENOCD_CONFIG_PATH=/home/ted/projects/psas/psas-avionics/lv3.1-recovery/controlSystem/RecoveryBoard/firmware/toolchain
	OPENOCD_CONFIG_FILE=oocd.cfg
        FIRMWARE_IMAGE=/home/ted/projects/zephyr-project/psas-ers-firmware/samples/hello-world/build/zephyr/zephyr.hex
	openocd \
        -f ${OPENOCD_CONFIG_PATH}/${OPENOCD_CONFIG_FILE} \
        -c "program ${FIRMWARE_IMAGE} verify reset exit"
}

function flash_with_psas_recovery_board_options
{
	OPENOCD_CONFIG_PATH=/home/ted/projects/psas/psas-avionics/lv3.1-recovery/controlSystem/RecoveryBoard/firmware/toolchain
	OPENOCD_CONFIG_FILE=oocd.cfg
        FIRMWARE_IMAGE=/home/ted/projects/zephyr-project/psas-ers-firmware/samples/hello-world/build/zephyr/zephyr.hex
	openocd \
        -f ${OPENOCD_CONFIG_PATH}/${OPENOCD_CONFIG_FILE} \
        -c "program ${FIRMWARE_IMAGE} verify reset"
}

function usage()
{
    echo "Call flash-manually.sh with:"
    echo "'w' to flash (write) an image to hardware"
    echo "'d' to flash and to start debugging server"
    echo "'psas' to flash using PSAS recovery board openocd invocation"
    echo ""
}

#-----------------------------------------------------------------------
# - SECTION - starting point akin to int main
#-----------------------------------------------------------------------

echo "Zephyr openocd flash wrapper starting"
echo "current dir is $PWD"

if [ "$1" == "w" ]; then
    echo "Calling openocd with options to flash and then exit openocd . . ."
    flash_only
elif [ "$1" == "d" ]; then
    echo "Calling openocd with options to flash, start and maintain a gdb server . . ."
    flash_and_enter_debugger
elif [ "$1" == "psas" ]; then
    echo "Calling openocd with PSAS options . . ."
    flash_with_psas_recovery_board_options
elif [ "$1" == "psas" && "$2" == "d"]; then
    flash_with_psas_options_and_debug
else
    usage
fi

exit $?
