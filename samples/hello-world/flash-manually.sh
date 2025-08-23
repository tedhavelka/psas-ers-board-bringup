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

function flash_without_openocd_shutdown()
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

function usage()
{
    echo "Call flash-manually.sh with:"
    echo "'w' to flash (write) an image to hardware"
    echo "'d' to flash and to start debugging server"
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
    flash_without_openocd_shutdown
else
    usage
fi

exit $?
