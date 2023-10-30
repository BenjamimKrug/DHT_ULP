#ifndef HULP_H
#define HULP_H

#include "esp32/ulp.h"
#include "hulp_macros.h"
#include "soc/rtc.h"
#include "esp32-hal-log.h"
#include "esp_system.h"
#include "esp_sleep.h"

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

  void hulp_set_start_delay(void);

  void hulp_peripherals_on(void);

  uint32_t hulp_get_fast_clk_freq(void);

#ifdef __cplusplus
}
#endif
#endif