#ifndef ADC_INIT
#define ADC_INIT

int32_t adc_init(void);

int32_t cmd_ers_read_adc_all(const struct shell *shell);

int32_t cmd_ers_read_adc_in0(const struct shell *shell);

#endif
