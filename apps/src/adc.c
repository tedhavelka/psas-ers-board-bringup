/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
// #include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/shell/shell.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ers_adc, CONFIG_ERS_ADC_LOG_LEVEL);

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ADC_THREAD_STACK_SIZE 512
#define ADC_THREAD_PRIORITY 3
#define ADC_READ_PERIOD_MS 5000

#undef DEV_ERS_ADC_REGULAR_REPORTING

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                 DT_SPEC_AND_COMMA)
};

struct k_thread adc_thread_data;

K_THREAD_STACK_DEFINE(adc_thread_stack, ADC_THREAD_STACK_SIZE);

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t cmd_ers_read_adc_in0(const struct shell *shell)
{
	shell_fprintf(shell, SHELL_NORMAL, "stub function for read ADC in0\n");
        return 0;
}

// TODO [ ] Add a function which calls an ERS module to store latest ADC
//   reading using Zephyr atomic access API.

// TODO [ ] Amend the "read ADC all" command to accept a range of channels,
//   to support the reading of one channel with the same routine.

int32_t cmd_ers_read_adc_all(const struct shell *shell)
{
        int32_t rc = 0;
        uint32_t count = 0;
        uint16_t buf;
        struct adc_sequence sequence = { 
                .buffer = &buf,
                /* buffer size in bytes, not number of samples */
                .buffer_size = sizeof(buf),
        };

        /* Configure channels individually prior to sampling. */
        for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
                if (!adc_is_ready_dt(&adc_channels[i])) {
                        // LOG_ERR("ADC controller device %s not ready\n", adc_channels[i].dev->name);
                        shell_fprintf(shell, SHELL_ERROR, "ADC controller device %s not ready\n",
                                     adc_channels[i].dev->name);
                        rc = -ENODEV;
                }

                rc = adc_channel_setup_dt(&adc_channels[i]);
                if (rc < 0) {
                        // LOG_ERR("Could not setup channel #%d (%d)\n", i, rc);
                        shell_fprintf(shell, SHELL_ERROR, "Could not setup channel #%d (%d)\n", i, rc);
                        rc = -EINVAL;
                }
        }

        // LOG_INF("ADC reading[%u]:\n", count++);
        shell_fprintf(shell, SHELL_NORMAL, "ADC reading[%u]:\n", count++);
        for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
        {
                int32_t val_mv;

                // LOG_INF("- %s, channel %d: ",
                shell_fprintf(shell, SHELL_NORMAL, "- %s, channel %d: ",
                             adc_channels[i].dev->name,
                             adc_channels[i].channel_id);

                (void)adc_sequence_init_dt(&adc_channels[i], &sequence);

                rc = adc_read_dt(&adc_channels[i], &sequence);
                if (rc < 0)
                {
                        // LOG_ERR("Could not read ADC channel, error (%d)\n", rc);
                        shell_fprintf(shell, SHELL_ERROR, "Could not read ADC channel, error (%d)\n", rc);
                        continue;
                }

                /*
                 * If using differential mode, the 16 bit value
                 * in the ADC sample buffer should be a signed 2's
                 * complement value.
                 */
                if (adc_channels[i].channel_cfg.differential)
                {
                        val_mv = (int32_t)((int16_t)buf);
                }
                else
                {
                        val_mv = (int32_t)buf;
                }

                // LOG_INF("%"PRId32, val_mv);
                shell_fprintf(shell, SHELL_NORMAL, "%"PRId32, val_mv);
                rc = adc_raw_to_millivolts_dt(&adc_channels[i],
                               &val_mv);
                /* conversion to mV may not be supported, skip if not */
                if (rc < 0)
                {
                        // LOG_WRN(" (value in mV not available)\n");
                        shell_fprintf(shell, SHELL_ERROR, " (value in mV not available)\n");
                }
                else
                {
                        // LOG_INF(" = %"PRId32" mV\n", val_mv);
                        shell_fprintf(shell, SHELL_NORMAL, " = %"PRId32" mV\n", val_mv);
                }
        }

	return rc;
}

void adc_thread_entry(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

        int32_t rc = 0;
        // uint32_t count = 0;
        uint16_t buf;
        struct adc_sequence sequence = {
                .buffer = &buf,
                /* buffer size in bytes, not number of samples */
                .buffer_size = sizeof(buf),
        };

        /* Configure channels individually prior to sampling. */
        for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
        {
                if (!adc_is_ready_dt(&adc_channels[i])) {
                        LOG_ERR("ADC controller device %s not ready\n", adc_channels[i].dev->name);
                        rc = -ENODEV;
                }

                rc = adc_channel_setup_dt(&adc_channels[i]);
                if (rc < 0) {
                        LOG_ERR("Could not setup channel #%d (%d)\n", i, rc);
                        rc = -EINVAL;
                }
        }

        while (1)
        {
#if DEV_ERS_ADC_REGULAR_REPORTING
                LOG_INF("ADC reading[%u]:\n", count++);
#endif
                for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
                {
                        int32_t val_mv;

#if DEV_ERS_ADC_REGULAR_REPORTING
                        LOG_INF("- %s, channel %d: ", adc_channels[i].dev->name,
                                 adc_channels[i].channel_id);
#endif

                        (void)adc_sequence_init_dt(&adc_channels[i], &sequence);

                        rc = adc_read_dt(&adc_channels[i], &sequence);
                        if (rc < 0)
                        {
#if DEV_ERS_ADC_REGULAR_REPORTING
                                LOG_ERR("Could not read ADC channel, error (%d)\n", rc);
#endif
                                continue;
                        }

                        /*
                         * If using differential mode, the 16 bit value
                         * in the ADC sample buffer should be a signed 2's
                         * complement value.
                         */
                        if (adc_channels[i].channel_cfg.differential)
                        {
                                val_mv = (int32_t)((int16_t)buf);
                        }
                        else
                        {
                                val_mv = (int32_t)buf;
                        }
#if DEV_ERS_ADC_REGULAR_REPORTING
                        LOG_INF("%"PRId32, val_mv);
#endif
                        rc = adc_raw_to_millivolts_dt(&adc_channels[i], &val_mv);
                        /* conversion to mV may not be supported, skip if not */
                        if (rc < 0)
                        {
#if DEV_ERS_ADC_REGULAR_REPORTING
                                LOG_WRN(" (value in mV not available)\n");
#endif
                        }
                        else
                        {
#if DEV_ERS_ADC_REGULAR_REPORTING
                                LOG_INF(" = %"PRId32" mV\n", val_mv);
#endif
                        }
                }
                k_sleep(K_MSEC(ADC_READ_PERIOD_MS));
        }
}

int32_t adc_init(void)
{
    int32_t rc = 0;

    k_tid_t adc_tid = k_thread_create(&adc_thread_data, adc_thread_stack,
                                 K_THREAD_STACK_SIZEOF(adc_thread_stack),
                                 adc_thread_entry, NULL, NULL, NULL,
                                 ADC_THREAD_PRIORITY, 0, K_NO_WAIT);
    if (!adc_tid)
    {
        LOG_ERR("ERROR spawning ADC thread\n");
    }
    else
    {
        LOG_INF("starting ADC thread . . .");
    }

    return rc;
}
