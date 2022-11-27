#ifndef HULP_MACRO_OPT_H
#define HULP_MACRO_OPT_H

#include "soc/soc.h"
#include "soc/rtc_io_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

#ifndef SOC_GPIO_PIN_COUNT
#define SOC_GPIO_PIN_COUNT 40
#endif
#ifndef SOC_RTCIO_PIN_COUNT
#define SOC_RTCIO_PIN_COUNT 18
#endif
#ifndef CONFIG_HULP_LABEL_AUTO_BASE
    #define CONFIG_HULP_LABEL_AUTO_BASE 60000
#endif
#ifndef CONFIG_HULP_FAST_CLK_CAL_CYCLES
    #define CONFIG_HULP_FAST_CLK_CAL_CYCLES 100
#endif

#define HULP_ULP_RESERVE_MEM CONFIG_ESP32_ULP_COPROC_RESERVE_MEM

#include "sdkconfig.h"


#if CONFIG_IDF_TARGET_ESP32
#if ESP_IDF_VERSION_MAJOR > 4
#include "esp_private/esp_clk.h"

#else
/**
 * @brief Pin function information for a single RTCIO pad's.
 *
 * This is an internal function of the driver, and is not usually useful
 * for external use.
 */
typedef struct {
  uint32_t reg;        /*!< Register of RTC pad, or 0 if not an RTC GPIO */
  uint32_t mux;        /*!< Bit mask for selecting digital pad or RTC pad */
  uint32_t func;       /*!< Shift of pad function (FUN_SEL) field */
  uint32_t ie;         /*!< Mask of input enable */
  uint32_t pullup;     /*!< Mask of pullup enable */
  uint32_t pulldown;   /*!< Mask of pulldown enable */
  uint32_t slpsel;     /*!< If slpsel bit is set, slpie will be used as pad input enabled signal in sleep mode */
  uint32_t slpie;      /*!< Mask of input enable in sleep mode */
  uint32_t slpoe;      /*!< Mask of output enable in sleep mode */
  uint32_t hold;       /*!< Mask of hold enable */
  uint32_t hold_force; /*!< Mask of hold_force bit for RTC IO in RTC_CNTL_HOLD_REG */
  uint32_t drv_v;      /*!< Mask of drive capability */
  uint32_t drv_s;      /*!< Offset of drive capability */
  int rtc_num;         /*!< GPIO number (corresponds to RTC pad) */
} rtc_io_desc_t;
#endif
#include "esp32/ulp.h"
#define HULP_ULP_RESERVE_MEM CONFIG_ESP32_ULP_COPROC_RESERVE_MEM

#else
#error "Target not supported"
#endif


#ifdef CONFIG_HULP_MACRO_OPTIMISATIONS

static const int s_hulp_rtc_io_num_map[SOC_GPIO_PIN_COUNT] = {
  RTCIO_GPIO0_CHANNEL,   //GPIO0
  -1,                    //GPIO1
  RTCIO_GPIO2_CHANNEL,   //GPIO2
  -1,                    //GPIO3
  RTCIO_GPIO4_CHANNEL,   //GPIO4
  -1,                    //GPIO5
  -1,                    //GPIO6
  -1,                    //GPIO7
  -1,                    //GPIO8
  -1,                    //GPIO9
  -1,                    //GPIO10
  -1,                    //GPIO11
  RTCIO_GPIO12_CHANNEL,  //GPIO12
  RTCIO_GPIO13_CHANNEL,  //GPIO13
  RTCIO_GPIO14_CHANNEL,  //GPIO14
  RTCIO_GPIO15_CHANNEL,  //GPIO15
  -1,                    //GPIO16
  -1,                    //GPIO17
  -1,                    //GPIO18
  -1,                    //GPIO19
  -1,                    //GPIO20
  -1,                    //GPIO21
  -1,                    //GPIO22
  -1,                    //GPIO23
  -1,                    //GPIO24
  RTCIO_GPIO25_CHANNEL,  //GPIO25
  RTCIO_GPIO26_CHANNEL,  //GPIO26
  RTCIO_GPIO27_CHANNEL,  //GPIO27
  -1,                    //GPIO28
  -1,                    //GPIO29
  -1,                    //GPIO30
  -1,                    //GPIO31
  RTCIO_GPIO32_CHANNEL,  //GPIO32
  RTCIO_GPIO33_CHANNEL,  //GPIO33
  RTCIO_GPIO34_CHANNEL,  //GPIO34
  RTCIO_GPIO35_CHANNEL,  //GPIO35
  RTCIO_GPIO36_CHANNEL,  //GPIO36
  RTCIO_GPIO37_CHANNEL,  //GPIO37
  RTCIO_GPIO38_CHANNEL,  //GPIO38
  RTCIO_GPIO39_CHANNEL,  //GPIO39
};



#define hulp_gtr(gpio_num) ((uint8_t)s_hulp_rtc_io_num_map[gpio_num])

#define RTC_WORD_OFFSET(x) ((uint16_t)((uint32_t*)(&(x)) - RTC_SLOW_MEM))

// See rtc_io_desc
static const rtc_io_desc_t s_hulp_rtc_io_desc[SOC_RTCIO_PIN_COUNT] = {
  /*REG                    MUX select                  function select              Input enable                Pullup                   Pulldown                 Sleep select                 Sleep input enable             PAD hold                  Pad force hold                    Mask of drive capability Offset                   gpio number */
  { RTC_IO_SENSOR_PADS_REG, RTC_IO_SENSE1_MUX_SEL_M, RTC_IO_SENSE1_FUN_SEL_S, RTC_IO_SENSE1_FUN_IE_M, 0, 0, RTC_IO_SENSE1_SLP_SEL_M, RTC_IO_SENSE1_SLP_IE_M, 0, RTC_IO_SENSE1_HOLD_M, RTC_CNTL_SENSE1_HOLD_FORCE_M, 0, 0, RTCIO_CHANNEL_0_GPIO_NUM },                                                                                                                      //36
  { RTC_IO_SENSOR_PADS_REG, RTC_IO_SENSE2_MUX_SEL_M, RTC_IO_SENSE2_FUN_SEL_S, RTC_IO_SENSE2_FUN_IE_M, 0, 0, RTC_IO_SENSE2_SLP_SEL_M, RTC_IO_SENSE2_SLP_IE_M, 0, RTC_IO_SENSE2_HOLD_M, RTC_CNTL_SENSE2_HOLD_FORCE_M, 0, 0, RTCIO_CHANNEL_1_GPIO_NUM },                                                                                                                      //37
  { RTC_IO_SENSOR_PADS_REG, RTC_IO_SENSE3_MUX_SEL_M, RTC_IO_SENSE3_FUN_SEL_S, RTC_IO_SENSE3_FUN_IE_M, 0, 0, RTC_IO_SENSE3_SLP_SEL_M, RTC_IO_SENSE3_SLP_IE_M, 0, RTC_IO_SENSE3_HOLD_M, RTC_CNTL_SENSE3_HOLD_FORCE_M, 0, 0, RTCIO_CHANNEL_2_GPIO_NUM },                                                                                                                      //38
  { RTC_IO_SENSOR_PADS_REG, RTC_IO_SENSE4_MUX_SEL_M, RTC_IO_SENSE4_FUN_SEL_S, RTC_IO_SENSE4_FUN_IE_M, 0, 0, RTC_IO_SENSE4_SLP_SEL_M, RTC_IO_SENSE4_SLP_IE_M, 0, RTC_IO_SENSE4_HOLD_M, RTC_CNTL_SENSE4_HOLD_FORCE_M, 0, 0, RTCIO_CHANNEL_3_GPIO_NUM },                                                                                                                      //39
  { RTC_IO_ADC_PAD_REG, RTC_IO_ADC1_MUX_SEL_M, RTC_IO_ADC1_FUN_SEL_S, RTC_IO_ADC1_FUN_IE_M, 0, 0, RTC_IO_ADC1_SLP_SEL_M, RTC_IO_ADC1_SLP_IE_M, 0, RTC_IO_ADC1_HOLD_M, RTC_CNTL_ADC1_HOLD_FORCE_M, 0, 0, RTCIO_CHANNEL_4_GPIO_NUM },                                                                                                                                        //34
  { RTC_IO_ADC_PAD_REG, RTC_IO_ADC2_MUX_SEL_M, RTC_IO_ADC2_FUN_SEL_S, RTC_IO_ADC2_FUN_IE_M, 0, 0, RTC_IO_ADC2_SLP_SEL_M, RTC_IO_ADC2_SLP_IE_M, 0, RTC_IO_ADC2_HOLD_M, RTC_CNTL_ADC2_HOLD_FORCE_M, 0, 0, RTCIO_CHANNEL_5_GPIO_NUM },                                                                                                                                        //35
  { RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_MUX_SEL_M, RTC_IO_PDAC1_FUN_SEL_S, RTC_IO_PDAC1_FUN_IE_M, RTC_IO_PDAC1_RUE_M, RTC_IO_PDAC1_RDE_M, RTC_IO_PDAC1_SLP_SEL_M, RTC_IO_PDAC1_SLP_IE_M, 0, RTC_IO_PDAC1_HOLD_M, RTC_CNTL_PDAC1_HOLD_FORCE_M, RTC_IO_PDAC1_DRV_V, RTC_IO_PDAC1_DRV_S, RTCIO_CHANNEL_6_GPIO_NUM },                                                            //25
  { RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_MUX_SEL_M, RTC_IO_PDAC2_FUN_SEL_S, RTC_IO_PDAC2_FUN_IE_M, RTC_IO_PDAC2_RUE_M, RTC_IO_PDAC2_RDE_M, RTC_IO_PDAC2_SLP_SEL_M, RTC_IO_PDAC2_SLP_IE_M, 0, RTC_IO_PDAC2_HOLD_M, RTC_CNTL_PDAC2_HOLD_FORCE_M, RTC_IO_PDAC2_DRV_V, RTC_IO_PDAC2_DRV_S, RTCIO_CHANNEL_7_GPIO_NUM },                                                            //26
  { RTC_IO_XTAL_32K_PAD_REG, RTC_IO_X32N_MUX_SEL_M, RTC_IO_X32N_FUN_SEL_S, RTC_IO_X32N_FUN_IE_M, RTC_IO_X32N_RUE_M, RTC_IO_X32N_RDE_M, RTC_IO_X32N_SLP_SEL_M, RTC_IO_X32N_SLP_IE_M, 0, RTC_IO_X32N_HOLD_M, RTC_CNTL_X32N_HOLD_FORCE_M, RTC_IO_X32N_DRV_V, RTC_IO_X32N_DRV_S, RTCIO_CHANNEL_8_GPIO_NUM },                                                                   //33
  { RTC_IO_XTAL_32K_PAD_REG, RTC_IO_X32P_MUX_SEL_M, RTC_IO_X32P_FUN_SEL_S, RTC_IO_X32P_FUN_IE_M, RTC_IO_X32P_RUE_M, RTC_IO_X32P_RDE_M, RTC_IO_X32P_SLP_SEL_M, RTC_IO_X32P_SLP_IE_M, 0, RTC_IO_X32P_HOLD_M, RTC_CNTL_X32P_HOLD_FORCE_M, RTC_IO_X32P_DRV_V, RTC_IO_X32P_DRV_S, RTCIO_CHANNEL_9_GPIO_NUM },                                                                   //32
  { RTC_IO_TOUCH_PAD0_REG, RTC_IO_TOUCH_PAD0_MUX_SEL_M, RTC_IO_TOUCH_PAD0_FUN_SEL_S, RTC_IO_TOUCH_PAD0_FUN_IE_M, RTC_IO_TOUCH_PAD0_RUE_M, RTC_IO_TOUCH_PAD0_RDE_M, RTC_IO_TOUCH_PAD0_SLP_SEL_M, RTC_IO_TOUCH_PAD0_SLP_IE_M, 0, RTC_IO_TOUCH_PAD0_HOLD_M, RTC_CNTL_TOUCH_PAD0_HOLD_FORCE_M, RTC_IO_TOUCH_PAD0_DRV_V, RTC_IO_TOUCH_PAD0_DRV_S, RTCIO_CHANNEL_10_GPIO_NUM },  // 4
  { RTC_IO_TOUCH_PAD1_REG, RTC_IO_TOUCH_PAD1_MUX_SEL_M, RTC_IO_TOUCH_PAD1_FUN_SEL_S, RTC_IO_TOUCH_PAD1_FUN_IE_M, RTC_IO_TOUCH_PAD1_RUE_M, RTC_IO_TOUCH_PAD1_RDE_M, RTC_IO_TOUCH_PAD1_SLP_SEL_M, RTC_IO_TOUCH_PAD1_SLP_IE_M, 0, RTC_IO_TOUCH_PAD1_HOLD_M, RTC_CNTL_TOUCH_PAD1_HOLD_FORCE_M, RTC_IO_TOUCH_PAD1_DRV_V, RTC_IO_TOUCH_PAD1_DRV_S, RTCIO_CHANNEL_11_GPIO_NUM },  // 0
  { RTC_IO_TOUCH_PAD2_REG, RTC_IO_TOUCH_PAD2_MUX_SEL_M, RTC_IO_TOUCH_PAD2_FUN_SEL_S, RTC_IO_TOUCH_PAD2_FUN_IE_M, RTC_IO_TOUCH_PAD2_RUE_M, RTC_IO_TOUCH_PAD2_RDE_M, RTC_IO_TOUCH_PAD2_SLP_SEL_M, RTC_IO_TOUCH_PAD2_SLP_IE_M, 0, RTC_IO_TOUCH_PAD2_HOLD_M, RTC_CNTL_TOUCH_PAD2_HOLD_FORCE_M, RTC_IO_TOUCH_PAD2_DRV_V, RTC_IO_TOUCH_PAD2_DRV_S, RTCIO_CHANNEL_12_GPIO_NUM },  // 2
  { RTC_IO_TOUCH_PAD3_REG, RTC_IO_TOUCH_PAD3_MUX_SEL_M, RTC_IO_TOUCH_PAD3_FUN_SEL_S, RTC_IO_TOUCH_PAD3_FUN_IE_M, RTC_IO_TOUCH_PAD3_RUE_M, RTC_IO_TOUCH_PAD3_RDE_M, RTC_IO_TOUCH_PAD3_SLP_SEL_M, RTC_IO_TOUCH_PAD3_SLP_IE_M, 0, RTC_IO_TOUCH_PAD3_HOLD_M, RTC_CNTL_TOUCH_PAD3_HOLD_FORCE_M, RTC_IO_TOUCH_PAD3_DRV_V, RTC_IO_TOUCH_PAD3_DRV_S, RTCIO_CHANNEL_13_GPIO_NUM },  //15
  { RTC_IO_TOUCH_PAD4_REG, RTC_IO_TOUCH_PAD4_MUX_SEL_M, RTC_IO_TOUCH_PAD4_FUN_SEL_S, RTC_IO_TOUCH_PAD4_FUN_IE_M, RTC_IO_TOUCH_PAD4_RUE_M, RTC_IO_TOUCH_PAD4_RDE_M, RTC_IO_TOUCH_PAD4_SLP_SEL_M, RTC_IO_TOUCH_PAD4_SLP_IE_M, 0, RTC_IO_TOUCH_PAD4_HOLD_M, RTC_CNTL_TOUCH_PAD4_HOLD_FORCE_M, RTC_IO_TOUCH_PAD4_DRV_V, RTC_IO_TOUCH_PAD4_DRV_S, RTCIO_CHANNEL_14_GPIO_NUM },  //13
  { RTC_IO_TOUCH_PAD5_REG, RTC_IO_TOUCH_PAD5_MUX_SEL_M, RTC_IO_TOUCH_PAD5_FUN_SEL_S, RTC_IO_TOUCH_PAD5_FUN_IE_M, RTC_IO_TOUCH_PAD5_RUE_M, RTC_IO_TOUCH_PAD5_RDE_M, RTC_IO_TOUCH_PAD5_SLP_SEL_M, RTC_IO_TOUCH_PAD5_SLP_IE_M, 0, RTC_IO_TOUCH_PAD5_HOLD_M, RTC_CNTL_TOUCH_PAD5_HOLD_FORCE_M, RTC_IO_TOUCH_PAD5_DRV_V, RTC_IO_TOUCH_PAD5_DRV_S, RTCIO_CHANNEL_15_GPIO_NUM },  //12
  { RTC_IO_TOUCH_PAD6_REG, RTC_IO_TOUCH_PAD6_MUX_SEL_M, RTC_IO_TOUCH_PAD6_FUN_SEL_S, RTC_IO_TOUCH_PAD6_FUN_IE_M, RTC_IO_TOUCH_PAD6_RUE_M, RTC_IO_TOUCH_PAD6_RDE_M, RTC_IO_TOUCH_PAD6_SLP_SEL_M, RTC_IO_TOUCH_PAD6_SLP_IE_M, 0, RTC_IO_TOUCH_PAD6_HOLD_M, RTC_CNTL_TOUCH_PAD6_HOLD_FORCE_M, RTC_IO_TOUCH_PAD6_DRV_V, RTC_IO_TOUCH_PAD6_DRV_S, RTCIO_CHANNEL_16_GPIO_NUM },  //14
  { RTC_IO_TOUCH_PAD7_REG, RTC_IO_TOUCH_PAD7_MUX_SEL_M, RTC_IO_TOUCH_PAD7_FUN_SEL_S, RTC_IO_TOUCH_PAD7_FUN_IE_M, RTC_IO_TOUCH_PAD7_RUE_M, RTC_IO_TOUCH_PAD7_RDE_M, RTC_IO_TOUCH_PAD7_SLP_SEL_M, RTC_IO_TOUCH_PAD7_SLP_IE_M, 0, RTC_IO_TOUCH_PAD7_HOLD_M, RTC_CNTL_TOUCH_PAD7_HOLD_FORCE_M, RTC_IO_TOUCH_PAD7_DRV_V, RTC_IO_TOUCH_PAD7_DRV_S, RTCIO_CHANNEL_17_GPIO_NUM },  //27
};

#define hulp_rtc_io_desc s_hulp_rtc_io_desc

#else  // CONFIG_HULP_MACRO_OPTIMISATIONS

#define hulp_gtr(gpio_num) ((uint8_t)rtc_io_number_get(gpio_num))

#define RTC_WORD_OFFSET(x) ({ \
  uint32_t* ptr_ = (uint32_t*)(&(x)); \
  TRY_STATIC_ASSERT((uint32_t)(ptr_) % sizeof(uint32_t) == 0, (Not aligned)); \
  TRY_STATIC_ASSERT((intptr_t)(ptr_) >= SOC_RTC_DATA_LOW && (intptr_t)(ptr_) < SOC_RTC_DATA_HIGH, (Not in RTC Slow Mem)); \
  ((uint16_t)(ptr_ - RTC_SLOW_MEM)); \
})

#define hulp_rtc_io_desc rtc_io_desc

#endif  // CONFIG_HULP_MACRO_OPTIMISATIONS

#endif /* HULP_MACRO_OPT_H */
