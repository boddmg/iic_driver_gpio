#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/io.h>
#include "lib-gpio.h"


volatile unsigned long *GPIOMODE;
volatile unsigned long *GPIO21_00_DIR;
volatile unsigned long *GPIO21_00_DATA;

volatile unsigned long *GPIO27_22_DIR;
volatile unsigned long *GPIO27_22_DATA;

void set_gpio_dir(unsigned int gpio_pin,enum gpio_dir dir)
{
    if(gpio_pin <= 21)
    {
        (dir == GPIO_INPUT) ? (*GPIO21_00_DIR &= ~(1<<gpio_pin)) : (*GPIO21_00_DIR |= (1<<gpio_pin));
    }else if(gpio_pin <=27)
    {
        gpio_pin -= 22;
        (dir == GPIO_INPUT) ? (*GPIO27_22_DIR &= ~(1<<gpio_pin)) : (*GPIO27_22_DIR |= (1<<gpio_pin));
    }
}

int read_gpio(unsigned int gpio_pin)
{
    if(gpio_pin <= 21)
    {
        return (*GPIO21_00_DATA & (1<<gpio_pin)) > 0;
    }else if(gpio_pin <=27)
    {
        gpio_pin -= 22;
        return (*GPIO27_22_DATA & (1<<gpio_pin)) > 0;
    }
    return -1;
}

void write_gpio(unsigned int gpio_pin,unsigned int state)
{
    if(gpio_pin <= 21)
    {
        (state == 0) ? (*GPIO21_00_DATA &= ~(1<<gpio_pin)) : (*GPIO21_00_DATA |= (1<<gpio_pin));
    }else if(gpio_pin <=27)
    {
        gpio_pin -= 22;
        (state == 0) ? (*GPIO27_22_DATA &= ~(1<<gpio_pin)) : (*GPIO27_22_DATA |= (1<<gpio_pin));
    }
}

void init_gpio(unsigned long gpio_mode)
{
    GPIOMODE = (volatile unsigned long *)ioremap(0x10000060, 4);
	GPIO21_00_DATA = (volatile unsigned long *)ioremap(0x10000620, 4);
	GPIO21_00_DIR = (volatile unsigned long *)ioremap(0x10000624, 4);
	GPIO27_22_DATA = (volatile unsigned long *)ioremap(0x10000670, 4);
	GPIO27_22_DIR = (volatile unsigned long *)ioremap(0x10000674, 4);

	*GPIOMODE |= gpio_mode;
}

void uninit_gpio()
{
    iounmap(GPIOMODE);

	iounmap(GPIO21_00_DIR);
	iounmap(GPIO21_00_DATA);

	iounmap(GPIO27_22_DIR);
	iounmap(GPIO27_22_DATA);
}
