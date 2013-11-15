#ifndef gpio_h
#define gpio_h

void gpio_led_init(int ledx);
void gpio_led_on(int ledx);
void gpio_led_off(int ledx);
void gpio_led_toggle(int ledx);
void gpio_button_test_w(int buttonx);
int gpio_button_test(int buttonx);
void gpio_button_init(int buttonx);
int gpio_read_c(int mask);
void gpio_write_c(int mask,int value);
void gpio_mode_c(int mask, int rezim);
int gpio_read_d(int mask);
void gpio_write_d(int mask,int value);
void gpio_mode_d(int mask, int rezim);
int gpio_read_e(int mask);
void gpio_write_e(int mask,int value);
void gpio_mode_e(int mask, int rezim);
void gpio_hard_init(void);
#endif