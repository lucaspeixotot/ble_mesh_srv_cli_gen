/**
 * @file ic_leds_pwm.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2019-01-04
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef _IC_LEDS_PWM_H_
#define _IC_LEDS_PWM_H_

#include <device.h>
#include <gpio.h>
#include <logging/sys_log.h>
#include <pwm.h>
#include <zephyr/types.h>

#define PWM_DRIVER DT_NORDIC_NRF_PWM_PWM_0_LABEL

#define PWM_CHANNEL0 LED0_GPIO_PIN
#define PWM_CHANNEL1 LED1_GPIO_PIN
#define PWM_CHANNEL2 LED2_GPIO_PIN
#define PWM_CHANNEL3 LED3_GPIO_PIN

typedef struct {
    struct device *device;
    u8_t pin;
    u16_t pulse_width;
    u16_t period;
    u8_t state;
    u8_t initiated;
} ic_leds_pwm_device_t;

int ic_leds_pwm_init_device(ic_leds_pwm_device_t *leds_pwm_device, char *pwm_driver, u8_t pin,
                            u16_t period);

int ic_leds_pwm_valid_pin_mask(u8_t pin);

int ic_leds_pwm_turn_led_on(ic_leds_pwm_device_t *leds_pwm_device);

int ic_leds_pwm_turn_led_off(ic_leds_pwm_device_t *leds_pwm_device);

int ic_leds_pwm_change_pulse(ic_leds_pwm_device_t *leds_pwm_device, u16_t pulse_width);

int ic_leds_pwm_set_pulse(ic_leds_pwm_device_t *leds_pwm_device);

#endif  // _IC_LEDS_PWM_H_
