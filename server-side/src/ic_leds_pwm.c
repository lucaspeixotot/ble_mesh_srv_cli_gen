/**
 * @file ic_leds.c
 * @Lucas Peixoto(lpdac@ic.ufal.br)
 * @brief
 * @version 0.1
 * @date 2018-12-17
 *
 * @copyright Copyright (c) 2018
 *
 */
#include "ic_leds_pwm.h"

int ic_leds_pwm_init_device(ic_leds_pwm_device_t *leds_pwm_device, char *pwm_driver, u8_t pin,
                            u16_t period)
{
    if (leds_pwm_device->initiated) {
        return -EBUSY;
    }

    if (ic_leds_pwm_valid_pin_mask(pin)) {
        return -EINVAL;
    }

    struct device *led_pwm_dev;

    led_pwm_dev = device_get_binding(pwm_driver);

    if (!led_pwm_dev) {
        printk("Cannot find %s!\n", pwm_driver);
        return -EINVAL;
    }
    leds_pwm_device->pin         = pin;
    leds_pwm_device->period      = period;
    leds_pwm_device->initiated   = 1;
    leds_pwm_device->state       = 1;
    leds_pwm_device->pulse_width = 0;
    leds_pwm_device->device      = led_pwm_dev;

    return 0;
}

int ic_leds_pwm_valid_pin_mask(u8_t pin)
{
    if (pin != PWM_CHANNEL0 && pin != PWM_CHANNEL1 && pin != PWM_CHANNEL2 && pin != PWM_CHANNEL3) {
        return -EINVAL;
    }
    return 0;
}

int ic_leds_pwm_turn_led_on(ic_leds_pwm_device_t *leds_pwm_device)
{
    if (!leds_pwm_device->initiated) {
        return -ENODEV;
    }

    if (leds_pwm_device->state) {
        return -EINVAL;
    }

    leds_pwm_device->state = 1;

    return 0;
}

int ic_leds_pwm_turn_led_off(ic_leds_pwm_device_t *leds_pwm_device)
{
    if (!leds_pwm_device->initiated) {
        return -ENODEV;
    }

    if (!leds_pwm_device->state) {
        return -EINVAL;
    }

    leds_pwm_device->state = 0;

    return 0;
}


int ic_leds_pwm_change_pulse(ic_leds_pwm_device_t *leds_pwm_device, u16_t pulse_width)
{
    if (!leds_pwm_device->initiated) {
        return -ENODEV;
    }

    if (pulse_width > leds_pwm_device->period || pulse_width < 0) {
        return -EINVAL;
    }

    leds_pwm_device->pulse_width = pulse_width;

    return 0;
}

int ic_leds_pwm_set_pulse(ic_leds_pwm_device_t *leds_pwm_device)
{
    if (!leds_pwm_device->initiated) {
        return -ENODEV;
    }

    if (leds_pwm_device->state) {
        int err = pwm_pin_set_usec(leds_pwm_device->device, leds_pwm_device->pin,
                                   leds_pwm_device->period, leds_pwm_device->pulse_width);
        if (err) {
            printk("PWM pin set fails\n");
            return err;
        }
    } else {
        int err = pwm_pin_set_usec(leds_pwm_device->device, leds_pwm_device->pin,
                                   leds_pwm_device->period, leds_pwm_device->period);
        if (err) {
            printk("PWM pin set fails\n");
            return err;
        }
    }
    return 0;
}