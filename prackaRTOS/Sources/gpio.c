//#include "gpio.h"
#include "derivative.h"

#define Pin0               1
#define Pin1               2
#define Pin2               4
#define Pin3               8
#define Pin4               16
#define Pin5               32
#define Pin6               64
#define Pin7               128
#define vystupMode         0 
#define vstupMode          1

void gpio_led_init(int ledx)
{
    PTFPE ^= ~ledx;
    PTFDD ^= ledx; 
}

void gpio_led_on(int ledx)
{
    PTFD &= ~ledx;
}

void gpio_led_off(int ledx)
{
    PTFD |= ledx;
}

void gpio_led_toggle(int ledx)
{
    PTFD ^= ledx;
}

void gpio_button_init(int buttonx)
{
    PTADD ^= ~buttonx;
    PTAPE ^= buttonx;
}

int gpio_button_test(int buttonx)
{   
    if((PTAD & buttonx) == 0)
        return 1;
    else
        return 0;
}

void gpio_button_test_w(int buttonx)
{
    while(gpio_button_test(buttonx) != 1);
}

int gpio_read_c(int mask)
{
    return PTCD & mask;
}

void gpio_write_c(int mask,int value)
{
    if(value == 1)
        PTCD |= mask;
    else
        PTCD &= ~mask;
}

void gpio_mode_c(int mask, int rezim)
{
    if(rezim == 1)//nastavení režimu na vstup
    {
        PTCDD &= ~mask;
        PTCPE |= mask;
    } else if(rezim == 0)//nastevení rezimu na výstup
    {
        PTCDD |= mask;
        PTCPE &= ~mask;
    }  
}

int gpio_read_d(int mask)
{
    return PTDD & mask;
}

void gpio_write_d(int mask,int value)
{
    if(value == 1)
        PTDD |= mask;
    else
        PTDD &= ~mask;
}

void gpio_mode_d(int mask, int rezim)
{
    if(rezim == 1)//nastavení režimu na vstup
    {
        PTDDD &= ~mask;
        PTDPE |= mask;
    } else if(rezim == 0)//nastevení rezimu na výstup
    {
        PTDDD |= mask;
        PTDPE &= ~mask;
    }  
}

int gpio_read_e(int mask)
{
    return PTED & mask;
}

void gpio_write_e(int mask,int value)
{
    if(value == 1)
        PTED |= mask;
    else
        PTED &= ~mask;
}

void gpio_mode_e(int mask, int rezim)
{
    if(rezim == 1)//nastavení režimu na vstup
    {
        PTEDD &= ~mask;
        PTEPE |= mask;
    } else if(rezim == 0)//nastevení rezimu na výstup
    {
        PTEDD |= mask;
        PTEPE &= ~mask;
    }  
}

void gpio_hard_init()
{
    
    gpio_mode_c(Pin2 | Pin3 | Pin4 | Pin5 | Pin6 | Pin7, 0);
    gpio_mode_d(Pin6 | Pin7, 0);
    //gpio_mode_d(Pin1 | Pin2 | Pin3 | Pin4 | Pin5 , 1);
    //gpio_mode_e(Pin3 | Pin4 | Pin5 , 1);
}