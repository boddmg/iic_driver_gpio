#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/types.h>

#include <asm/uaccess.h>
#include "lib-gpio.h"

typedef struct iic_struct{
    u_int32_t sda_pin;
    u_int32_t scl_pin;
    u_int32_t udelay;
}iic_struct_typedef;

iic_struct_typedef iic_struct = {
    .sda_pin = 1,
    .scl_pin = 2,
    .udelay = 1000
};

static struct class *iic_gpio_custom_class;


void iic_gpio_custom_write_bits(u_int32_t data,u_int32_t size)
{
    int i=0;
    if(size > 32)
    {
        size = 32;
    }

    for(i=0;i<size;++i)
    {
        write_gpio(iic_struct.scl_pin, 0 );
        write_gpio(iic_struct.sda_pin, data&0x01 );
        data >>= 1;
        udelay(iic_struct.udelay);
        write_gpio(iic_struct.scl_pin, 1 );
        udelay(iic_struct.udelay);
        write_gpio(iic_struct.scl_pin, 0 );
    }
}

void iic_gpio_custom_end(void)
{
    set_gpio_dir(iic_struct.sda_pin,GPIO_OUTPUT);
    set_gpio_dir(iic_struct.scl_pin,GPIO_OUTPUT);
    write_gpio(iic_struct.scl_pin,0);
    write_gpio(iic_struct.sda_pin,0);
    udelay(iic_struct.udelay);
    write_gpio(iic_struct.scl_pin,1);
    write_gpio(iic_struct.sda_pin,0);
    udelay(iic_struct.udelay);
    write_gpio(iic_struct.sda_pin,1);
    write_gpio(iic_struct.scl_pin,1);
    udelay(iic_struct.udelay);
}


int32_t iic_gpio_custom_start(u_int8_t address, u_int8_t is_write)
{

    int32_t has_ack = 0;
    write_gpio(iic_struct.scl_pin,1);
    write_gpio(iic_struct.sda_pin,1);
    udelay(iic_struct.udelay);
    write_gpio(iic_struct.sda_pin,0);
    udelay(iic_struct.udelay);
    write_gpio(iic_struct.scl_pin,0);

    address <<= 1;
    if(!is_write) // bit8 = 0 when the master want to write data to slave.
    {
        address |= 0x01;      
    }
    iic_gpio_custom_write_bits(address,8);

    write_gpio(iic_struct.scl_pin,0);
    udelay(iic_struct.udelay);
    write_gpio(iic_struct.scl_pin,1);
    write_gpio(iic_struct.sda_pin,1);
    set_gpio_dir(iic_struct.sda_pin,GPIO_INPUT);
    udelay(iic_struct.udelay);
    has_ack = !read_gpio(iic_struct.sda_pin); // bit9 = 0 when slave has the ack.
    set_gpio_dir(iic_struct.sda_pin,GPIO_OUTPUT);
    return has_ack;
}

int32_t iic_gpio_custom_read_byte(void)
{

    int32_t data = 0;
    u_int32_t i = 0;

    set_gpio_dir(iic_struct.sda_pin,GPIO_INPUT);

    write_gpio(iic_struct.sda_pin,1);
    write_gpio(iic_struct.scl_pin,0);
    udelay(iic_struct.udelay);

    for(i=0;i<8;++i)
    {
        write_gpio(iic_struct.scl_pin, 1);
        udelay(iic_struct.udelay);
        data <<= 1;
        data |= read_gpio(iic_struct.sda_pin);
        write_gpio(iic_struct.scl_pin, 0);
        udelay(iic_struct.udelay);
    }

    write_gpio(iic_struct.sda_pin,1);
    write_gpio(iic_struct.scl_pin,0);
    set_gpio_dir(iic_struct.sda_pin,GPIO_OUTPUT);

    return data;
}

static int iic_gpio_custom_open(struct inode *inode, struct file *file)
{
    set_gpio_dir(iic_struct.sda_pin,GPIO_OUTPUT);
    set_gpio_dir(iic_struct.scl_pin,GPIO_OUTPUT);

    write_gpio(iic_struct.sda_pin,1);
    write_gpio(iic_struct.scl_pin,1);

	return 0;
}

static ssize_t iic_gpio_custom_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{

    printk("Function %s called.\n", __FUNCTION__);
    return 0;

}

static ssize_t iic_gpio_custom_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{

    int i=0;

	if(size == 0)
	{
		return size;
	}

	printk("Function %s called, data:", __FUNCTION__);
    iic_gpio_custom_start(0x23, 1);
	for(i =0 ; i < size; ++i)
	{
		printk("%c",buf[i]);
        iic_gpio_custom_write_bits(buf[i], 8);
	}

    iic_gpio_custom_end();

    write_gpio(iic_struct.sda_pin,1);
    write_gpio(iic_struct.scl_pin,1);

    set_gpio_dir(iic_struct.sda_pin,GPIO_INPUT);
    set_gpio_dir(iic_struct.scl_pin,GPIO_INPUT);

    printk("SDA:%d;SCL:%d\n",read_gpio(iic_struct.sda_pin),read_gpio(iic_struct.scl_pin));

    set_gpio_dir(iic_struct.sda_pin,GPIO_OUTPUT);
    set_gpio_dir(iic_struct.scl_pin,GPIO_OUTPUT);

	return size;

}

static long iic_gpio_custom_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	switch(cmd)
	{
		default:
			break;
	}
	return 0;

}

static struct file_operations iic_gpio_custom_fops = {
	.owner   			= THIS_MODULE,
	.open    			= iic_gpio_custom_open,
	.read    			= iic_gpio_custom_read,
	.write    			= iic_gpio_custom_write,
	.unlocked_ioctl	= iic_gpio_custom_unlocked_ioctl,
};

int major;
static int __init iic_gpio_custom_init(void)
{
	major = register_chrdev(0, "iic_gpio_custom", &iic_gpio_custom_fops);

	iic_gpio_custom_class = class_create(THIS_MODULE, "iic_gpio_custom");
	device_create(iic_gpio_custom_class, NULL, MKDEV(major, 0), NULL, "iic_gpio_custom");

    init_gpio(1<<0); // 1<<0 is set the gpio1 gpio2 as the gpio.

    set_gpio_dir(iic_struct.sda_pin,GPIO_OUTPUT);
    set_gpio_dir(iic_struct.scl_pin,GPIO_OUTPUT);

	return 0;
}

static void __exit iic_gpio_custom_exit(void)
{
	unregister_chrdev(major, "iic_gpio_custom");
	device_destroy(iic_gpio_custom_class, MKDEV(major, 0));
	class_destroy(iic_gpio_custom_class);
    uninit_gpio();
}

module_init(iic_gpio_custom_init);
module_exit(iic_gpio_custom_exit);

MODULE_LICENSE("GPL");

