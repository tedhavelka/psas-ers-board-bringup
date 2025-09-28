/*
 * Copyright (c) 2018 Alexander Wachter
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/devicetree.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(can_counter, CONFIG_SAMPLE_CAN_COUNTER_LOG_LEVEL);

#define RX_THREAD_STACK_SIZE 512
#define RX_THREAD_PRIORITY 2
K_THREAD_STACK_DEFINE(rx_thread_stack, RX_THREAD_STACK_SIZE);
struct k_thread rx_thread_data;

#define STATE_POLL_THREAD_STACK_SIZE 512
#define STATE_POLL_THREAD_PRIORITY 2
K_THREAD_STACK_DEFINE(poll_state_stack, STATE_POLL_THREAD_STACK_SIZE);
struct k_thread poll_state_thread_data;

#define LED_MSG_ID 0x10
#define COUNTER_MSG_ID 0x12345
#define SET_LED 1
#define RESET_LED 0

#define SLEEP_TIME K_MSEC(250)

/**
 * @note CAN frame ids for ERS:  while ERS sender won't listen for
 *   ERS drogue board CAN heartbeat nor ERS main board heartbeat, we
 *   include these message ids with the plan that one firmware project
 *   will include a build time flag to select the ids needed by each of
 *   three ERS firmwares, which are much more alike than different.
 */

// clang-format off
#define MSG_ID_TELEMETRUM_SENDER   0x700
#define MSG_ID_DROGUE_HEARTBEAT    0x710
#define MSG_ID_MAIN_HEARTBEAT      0x720
#define MSG_ID_UNLOCK_DROGUE_CHUTE 0x100
#define MSG_ID_UNLOCK_MAIN_CHUTE   0x200
// clang-format on

// TODO [ ] Come up with some pound defines or similar to select a status
//  message ID for the ERS firmware variant needed:

// #if ERS_BOARD_VARIANT == ERS_DROGUE_CHUTE_CONTROLLER . . .
#define MSG_ID_STATUS_AND_HEARTBEAT MSG_ID_DROGUE_HEARTBEAT

#define HEARTBEAT_PERIOD_S 1
#define CAN_BUS_CHECK_PER_S 2

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

struct k_work_poll change_led_work;
struct k_work state_change_work;
enum can_state current_state;
struct can_bus_err_cnt current_err_cnt;

// TODO [ ] Review whether both of these message queues needed:
CAN_MSGQ_DEFINE(change_led_msgq, 2);
CAN_MSGQ_DEFINE(counter_msgq, 2);

// TODO [ ] Remove this and the code associated with this kernel poll event structure:
static struct k_poll_event change_led_events[1] = {
	K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
					K_POLL_MODE_NOTIFY_ONLY,
					&change_led_msgq, 0)
};

enum ers_state_var_indeces {
	IDX_TELEMETRUM_STATE,
	IDX_BATT_READ,
	IDX_BATT_OK,
	IDX_SHORE_POW_STATUS,
	IDX_CAN_BUS_OK,
	IDX_ERS_STATUS,
	IDX_ROCKET_READY,
	IDX_RESERVED,
	IDX_STATE_VAR_LAST_ELEMENT
};

static uint8_t ers_state_vars_fs[IDX_STATE_VAR_LAST_ELEMENT] = {0};



//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

void clear_flag_can_ok_work_handler(struct k_work *work)
{
    /* do the processing that needs to be done periodically */
    // LOG_INF("CAN module timer expired!");
    // TODO [ ] Add application state flag to keeper module to hold "CAN bus ok" status
}

K_WORK_DEFINE(clear_flag_can_ok_work, clear_flag_can_ok_work_handler);

void my_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&clear_flag_can_ok_work);
}

K_TIMER_DEFINE(my_timer, my_timer_handler, NULL);

/**
 * @brief IRQ callback to provide to can_send() API.
 *
 * @note Different calls to can_send() may likely need distinct callbacks.
 *
 * @note This detail learned from Zephyr sample app, by Alexander Wachter
 *   from Zephyr 3.7.1, copyright 2018.
 */

void tx_irq_callback(const struct device *dev, int error, void *arg)
{
        char *sender = (char *)arg;

        ARG_UNUSED(dev);

        if (error != 0) {
                printf("Callback! error-code: %d\nSender: %s\n",
                       error, sender);
        }
}

void prep_and_send_status_frame_work_handler(struct k_work *work)
{
	LOG_INF("preparing status message, data length is %d", sizeof(ers_state_vars_fs));

        struct can_frame ers_status_frame = {
                .flags = 0,
                .id = MSG_ID_STATUS_AND_HEARTBEAT,
                .dlc = sizeof(ers_state_vars_fs)
        };

	ers_state_vars_fs[IDX_TELEMETRUM_STATE] = 0;
	ers_state_vars_fs[IDX_BATT_READ] = 0;
	ers_state_vars_fs[IDX_BATT_OK] = 0;
	ers_state_vars_fs[IDX_SHORE_POW_STATUS] = 0;
	ers_state_vars_fs[IDX_CAN_BUS_OK] = 0;
	ers_state_vars_fs[IDX_ERS_STATUS] = 0;
	ers_state_vars_fs[IDX_ROCKET_READY] = 0;
	ers_state_vars_fs[IDX_RESERVED] = 0;

	memcpy(ers_status_frame.data, ers_state_vars_fs, sizeof(ers_state_vars_fs));

	can_send(can_dev, &ers_status_frame, K_FOREVER,
		 tx_irq_callback,
		 "ERS status frame");
}

K_WORK_DEFINE(prep_and_send_status_frame_work, prep_and_send_status_frame_work_handler);

void heartbeat_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&prep_and_send_status_frame_work);
}

K_TIMER_DEFINE(heartbeat_timer, heartbeat_timer_handler, NULL);


void rx_thread_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	const struct can_filter filter_sender_heartbeat = {
		.flags = CAN_FILTER_IDE,
		.id = MSG_ID_TELEMETRUM_SENDER,
		.mask = CAN_EXT_ID_MASK
	};

	const struct can_filter filter_drogue_heartbeat = {
		.flags = CAN_FILTER_IDE,
		.id = MSG_ID_DROGUE_HEARTBEAT,
		.mask = CAN_EXT_ID_MASK
	};

	const struct can_filter filter_main_heartbeat = {
		.flags = CAN_FILTER_IDE,
		.id = MSG_ID_MAIN_HEARTBEAT,
		.mask = CAN_EXT_ID_MASK
	};

	struct can_frame frame;
	int filter_id;

	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_sender_heartbeat);
	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_drogue_heartbeat);
	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_main_heartbeat);
	LOG_INF("Counter filter id: %d\n", filter_id);

	while (1) {
		k_msgq_get(&counter_msgq, &frame, K_FOREVER);

		if (IS_ENABLED(CONFIG_CAN_ACCEPT_RTR) && (frame.flags & CAN_FRAME_RTR) != 0U) {
			LOG_INF("M1");
			continue;
		}

#if 1
		if (frame.id == MSG_ID_TELEMETRUM_SENDER) {
			k_timer_start(&my_timer, K_SECONDS(2), K_SECONDS(2));
			LOG_INF("RX %X - telemetrum heartbeat", frame.id);
		}
#endif

#if 0
		switch (frame.id)
		{
		case MSG_ID_TELEMETRUM_SENDER:
			LOG_INF("RX %X - telemetrum heartbeat", frame.id);
			break;
		case MSG_ID_DROGUE_HEARTBEAT:
			LOG_INF("RX %X - drogue chute heartbeat", frame.id);
			break;
		case MSG_ID_MAIN_HEARTBEAT:
			LOG_INF("RX %X - main chute heartbeat", frame.id);
			break;
		case MSG_ID_UNLOCK_DROGUE_CHUTE:
			LOG_INF("RX %X - unlock drogue chute", frame.id);
			break;
		case MSG_ID_UNLOCK_MAIN_CHUTE:
			LOG_INF("RX %X - unlock main chute", frame.id);
			break;
		default:
		}
#endif
	}
}

void change_led_work_handler(struct k_work *work)
{
	struct can_frame frame;
	int ret;

	while (k_msgq_get(&change_led_msgq, &frame, K_NO_WAIT) == 0) {
		if (IS_ENABLED(CONFIG_CAN_ACCEPT_RTR) && (frame.flags & CAN_FRAME_RTR) != 0U) {
			continue;
		}

		if (led.port == NULL) {
			LOG_INF("LED %s\n", frame.data[0] == SET_LED ? "ON" : "OFF");
		} else {
			gpio_pin_set(led.port, led.pin, frame.data[0] == SET_LED ? 1 : 0);
		}
	}

	ret = k_work_poll_submit(&change_led_work, change_led_events,
				 ARRAY_SIZE(change_led_events), K_FOREVER);
	if (ret != 0) {
		LOG_ERR("Failed to resubmit msgq polling: %d", ret);
	}
}

char *state_to_str(enum can_state state)
{
	switch (state) {
	case CAN_STATE_ERROR_ACTIVE:
		return "error-active";
	case CAN_STATE_ERROR_WARNING:
		return "error-warning";
	case CAN_STATE_ERROR_PASSIVE:
		return "error-passive";
	case CAN_STATE_BUS_OFF:
		return "bus-off";
	case CAN_STATE_STOPPED:
		return "stopped";
	default:
		return "unknown";
	}
}

void poll_state_thread(void *unused1, void *unused2, void *unused3)
{
	struct can_bus_err_cnt err_cnt = {0, 0};
	struct can_bus_err_cnt err_cnt_prev = {0, 0};
	enum can_state state_prev = CAN_STATE_ERROR_ACTIVE;
	enum can_state state;
	int err;

	while (1) {
		err = can_get_state(can_dev, &state, &err_cnt);
		if (err != 0) {
			LOG_ERR("Failed to get CAN controller state: %d", err);
			k_sleep(K_MSEC(100));
			continue;
		}

		if (err_cnt.tx_err_cnt != err_cnt_prev.tx_err_cnt ||
		    err_cnt.rx_err_cnt != err_cnt_prev.rx_err_cnt ||
		    state_prev != state) {

			err_cnt_prev.tx_err_cnt = err_cnt.tx_err_cnt;
			err_cnt_prev.rx_err_cnt = err_cnt.rx_err_cnt;
			state_prev = state;
			LOG_INF("state: %s\n"
			       "rx error count: %d\n"
			       "tx error count: %d\n",
			       state_to_str(state),
			       err_cnt.rx_err_cnt, err_cnt.tx_err_cnt);
		} else {
			k_sleep(K_MSEC(100));
		}
	}
}

void state_change_work_handler(struct k_work *work)
{
	LOG_INF("State Change ISR\nstate: %s\n"
	       "rx error count: %d\n"
	       "tx error count: %d\n",
		state_to_str(current_state),
		current_err_cnt.rx_err_cnt, current_err_cnt.tx_err_cnt);
}

void state_change_callback(const struct device *dev, enum can_state state,
			   struct can_bus_err_cnt err_cnt, void *user_data)
{
	struct k_work *work = (struct k_work *)user_data;

	ARG_UNUSED(dev);

	current_state = state;
	current_err_cnt = err_cnt;
	k_work_submit(work);
}

int32_t ers_init_can(void)
{
	int32_t rc = 0;

	const struct can_filter change_led_filter = {
		.flags = 0U,
		.id = LED_MSG_ID,
		.mask = CAN_STD_ID_MASK
	};

	k_tid_t rx_tid, get_state_tid;
	int ret;

	if (!device_is_ready(can_dev)) {
		LOG_ERR("CAN: Device %s not ready.\n", can_dev->name);
		return -ENODEV;
	}

// TODO [ ] Learn how to set CAN bus bit rate in Zephyr app.  Following API
//   did not seem to be available:
#if 0
	ret = can_set_bitrate_data(can_dev, 500000);
	if (ret != 0) {
		LOG_ERR("Failed to set CAN bitrate, error %d]", ret);
		return 0;
	}
#endif

	ret = can_start(can_dev);
	if (ret != 0) {
		LOG_ERR("Error starting CAN controller [%d]", ret);
		return -EAGAIN;
	}

// TODO [ ] Replace '2' with symbol to indicate "ERS CAN bus ok" timeout period in seconds:
/* start a periodic timer that expires once every second */
	k_timer_start(&my_timer, K_SECONDS(CAN_BUS_CHECK_PER_S), K_SECONDS(CAN_BUS_CHECK_PER_S));

	k_timer_start(&heartbeat_timer, K_SECONDS(HEARTBEAT_PERIOD_S), K_SECONDS(HEARTBEAT_PERIOD_S));

// TODO [ ] Remove LED related sample code:
	if (led.port != NULL) {
		if (!gpio_is_ready_dt(&led)) {
			LOG_ERR("LED: Device %s not ready.\n",
			       led.port->name);
			return 0;
		}
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_HIGH);
		if (ret < 0) {
			LOG_ERR("Error setting LED pin to output mode [%d]",
			       ret);
			led.port = NULL;
		}
	}
	else
	{
		LOG_WRN("- DEV 0916 - During init stage led.port found null!");
	}

	k_work_init(&state_change_work, state_change_work_handler);
	k_work_poll_init(&change_led_work, change_led_work_handler);

	ret = can_add_rx_filter_msgq(can_dev, &change_led_msgq, &change_led_filter);
	if (ret == -ENOSPC) {
		LOG_ERR("Error, no filter available!\n");
		return 0;
	}

	LOG_INF("Change LED filter ID: %d\n", ret);

	ret = k_work_poll_submit(&change_led_work, change_led_events,
				 ARRAY_SIZE(change_led_events), K_FOREVER);
	if (ret != 0) {
		LOG_ERR("Failed to submit msgq polling: %d", ret);
		return 0;
	}

	rx_tid = k_thread_create(&rx_thread_data, rx_thread_stack,
				 K_THREAD_STACK_SIZEOF(rx_thread_stack),
				 rx_thread_entry, NULL, NULL, NULL,
				 RX_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!rx_tid) {
		LOG_ERR("ERROR spawning rx thread\n");
	}

	get_state_tid = k_thread_create(&poll_state_thread_data,
					poll_state_stack,
					K_THREAD_STACK_SIZEOF(poll_state_stack),
					poll_state_thread, NULL, NULL, NULL,
					STATE_POLL_THREAD_PRIORITY, 0,
					K_NO_WAIT);
	if (!get_state_tid) {
		LOG_ERR("ERROR spawning poll_state_thread\n");
	}

	can_set_state_change_callback(can_dev, state_change_callback, &state_change_work);

	LOG_INF("Finished init.\n");

#if 0
	while (1) {
		change_led_frame.data[0] = toggle++ & 0x01 ? SET_LED : RESET_LED;
		/* This sending call is none blocking. */
		can_send(can_dev, &change_led_frame, K_FOREVER,
			 tx_irq_callback,
			 "LED change");
		k_sleep(SLEEP_TIME);

		UNALIGNED_PUT(sys_cpu_to_be16(counter),
			      (uint16_t *)&counter_frame.data[0]);
		counter++;
		/* This sending call is blocking until the message is sent. */
		can_send(can_dev, &counter_frame, K_MSEC(100), NULL, NULL);
		k_sleep(SLEEP_TIME);
	}
#endif

	return rc;
}
