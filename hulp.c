#ifndef HULP_C
#define HULP_C
#include "hulp.h"

esp_err_t hulp_configure_pin(gpio_num_t pin, rtc_gpio_mode_t mode, gpio_pull_mode_t pull_mode, uint32_t level) {
  if (ESP_OK != rtc_gpio_set_direction(pin, RTC_GPIO_MODE_DISABLED) || ESP_OK != rtc_gpio_init(pin) || ESP_OK != gpio_set_pull_mode(pin, pull_mode) || ESP_OK != rtc_gpio_set_level(pin, level) || ESP_OK != rtc_gpio_set_direction(pin, mode)) {
    log_e("error - %d (%d, %d, %i)", pin, mode, pull_mode, level);
    return ESP_FAIL;
  }
  return ESP_OK;
}


void hulp_peripherals_on(void) {
  hulp_set_start_delay();
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
}

uint32_t hulp_get_fast_clk_freq(void) {
#ifdef CONFIG_HULP_USE_APPROX_FAST_CLK
  return (uint32_t)RTC_FAST_CLK_FREQ_APPROX;
#else
  const bool clk_8m_enabled = rtc_clk_8m_enabled();
  const bool clk_8md256_enabled = rtc_clk_8md256_enabled();
  if (!clk_8m_enabled || !clk_8md256_enabled) {
    rtc_clk_8m_enable(true, true);
  }
  uint32_t ret = (uint32_t)(1000000ULL * (1 << RTC_CLK_CAL_FRACT) * 256 / rtc_clk_cal(RTC_CAL_8MD256, CONFIG_HULP_FAST_CLK_CAL_CYCLES));
  if (!clk_8m_enabled || !clk_8md256_enabled) {
    rtc_clk_8m_enable(clk_8m_enabled, clk_8md256_enabled);
  }
  return ret;
#endif
}

void hulp_set_start_delay(void) {
  /*
      ULP is not officially supported if RTC peripherals domain is powered on,
     however this is often desirable. The only observed bug is that, in deep
     sleep, the ULP may return to sleep very soon after starting up (typically
     after just the first instruction), resulting in an apparent doubled wakeup
     period. To fix this, the ULP start wait needs to be increased slightly
     (from the default 0x10). Note that ulp_set_wakeup_period adjusts for this
     setting so timing should be unaffected. There should also, therefore, be no
     side effects of setting this when unnecessary (ie. RTC peripherals not
     forced on).
  */
  REG_SET_FIELD(RTC_CNTL_TIMER2_REG, RTC_CNTL_ULPCP_TOUCH_START_WAIT, 0x20);
}

#endif