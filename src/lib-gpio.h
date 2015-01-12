#ifndef _LIB_GPIO_H__
#define _LIB_GPIO_H__

enum gpio_dir{
    GPIO_INPUT = 0,GPIO_OUTPUT
};

void init_gpio(unsigned long gpio_mode);
void set_gpio_dir(unsigned int gpio_pin,enum gpio_dir dir);
int read_gpio(unsigned int gpio_pin);
void write_gpio(unsigned int gpio_pin,unsigned int state);
void uninit_gpio(void);

#endif

