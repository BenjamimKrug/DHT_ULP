#ifndef HULP_H
#define HULP_H

#include "esp32/ulp.h"
#include "hulp_macros.h"
#include "soc/rtc.h"
#include "esp32-hal-log.h"
#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif
  typedef union {
    struct {
      uint32_t data : 16;
      uint32_t reg_off : 2;
      uint32_t st : 3;
      uint32_t pc : 11;
    };
    struct {
      uint16_t val;
      uint16_t meta;
    };
    struct {
      uint8_t val_bytes[2];
      uint8_t meta_bytes[2];
    };
    struct {
      uint8_t bytes[4];
    };
    ulp_insn_t insn;
    uint32_t word;
  } ulp_var_t;
  _Static_assert(sizeof(ulp_var_t) == 4, "ulp_var_t size should be 4 bytes");


  esp_err_t hulp_configure_pin(gpio_num_t pin, rtc_gpio_mode_t mode, gpio_pull_mode_t pull_mode, uint32_t level);

  static void hulp_set_start_delay(void) {
    /*
        ULP is not officially supported if RTC peripherals domain is powered on, however this is often desirable.
        The only observed bug is that, in deep sleep, the ULP may return to sleep very soon after starting up (typically after
        just the first instruction), resulting in an apparent doubled wakeup period.
        To fix this, the ULP start wait needs to be increased slightly (from the default 0x10).
        Note that ulp_set_wakeup_period adjusts for this setting so timing should be unaffected. There should also, therefore,
        be no side effects of setting this when unnecessary (ie. RTC peripherals not forced on).
    */
    REG_SET_FIELD(RTC_CNTL_TIMER2_REG, RTC_CNTL_ULPCP_TOUCH_START_WAIT, 0x20);
  }

  void hulp_peripherals_on(void);

  uint32_t hulp_get_fast_clk_freq(void);

#ifdef __cplusplus
}
#endif
#endif