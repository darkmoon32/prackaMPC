#include "derivative.h"
#include "gpio.h"

#define Pin0               1
#define Pin1               2
#define Pin2               4
#define Pin3               8
#define Pin4               16
#define Pin5               32
#define Pin6               64
#define Pin7               128

//smer otaceni volen pomoci hodnoty int
//1 - otaceni leva
//0 - neotacej
//-1 - otaceni prava

void driver_buben_smer(int smer)
{
    if(smer == 1)
    {
        gpio_write_c(Pin6,0);
        gpio_write_c(Pin7,1);
    }else if(smer == 0)
    {
        gpio_write_c(Pin6,0);
        gpio_write_c(Pin7,0);
    } else if(smer == -1)
    {
        gpio_write_c(Pin6,1);
        gpio_write_c(Pin7,0);
    }
        
}
//zmìní rychlost otáèení bubnu
//1 rychlé toèení
//0 pomalé toèení
void driver_buben_rychlost(int rychlost)
{
    if(rychlost == 1)
    {
        gpio_write_c(Pin4,1);
    } else
    {
        gpio_write_c(Pin6,0);
    }
}
void driver_pracka_vypust(int vypust)
{
    if(vypust == 1)
    {
        gpio_write_c(Pin2,0);
        gpio_write_d(Pin7,1);
    } else
    {
        gpio_write_d(Pin7,0);
    }
}
    
void driver_pracka_napust(int napust)
{
    if(napust == 1)
    {
        gpio_write_c(Pin2,1);
        gpio_write_d(Pin7,0);
    } else
    {
        gpio_write_c(Pin2,0);
    }
}
int driver_pracka_teplota(void)
{
    if(gpio_read_d(Pin1))
        return 40;
    else if(gpio_read_d(Pin4))
        return 60;
    else if(gpio_read_d(Pin5))
        return 30;
    else if(gpio_read_e(Pin5))
        return 90;
    else
        return 0;
}
int driver_pracka_hladina(void)
{
    if(gpio_read_d(Pin3))
        return 100;
    else if(gpio_read_e(Pin4))
        return 50;
    else
        return 0;
}

void driver_pracka_topeni(int top)
{
    if(top == 1)
        gpio_write_c(Pin3,1);
    else
        gpio_write_c(Pin3,0);
}