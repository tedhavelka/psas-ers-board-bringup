#!/bin/bash

# /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/bin/openocd -f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/interface/stlink.cfg -f /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts/target/stm32f0x.cfg -s /opt/zephyr-sdk-0.16.5-1/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts  '-c init' '-c targets' -c 'reset init' -c 'flash write_image erase /home/ted/projects/zephyr-project/psas-ers-firmware/samples/hello-world/build/zephyr/zephyr.hex' -c 'reset run' -c shutdown

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

# flash_only
flash_without_openocd_shutdown

exit $?
