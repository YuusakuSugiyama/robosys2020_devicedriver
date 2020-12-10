// SPDX-License-Identifer: GPL-3.0
/*
 * Copyright (C) 2020 Hikaru Jitsukawa. All rights reserved.
 */

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("Ryuichi Ueda and Jitsukawa Hikaru");
MODULE_DESCRIPTION("driver for control LED with button");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

#define REG_ADDR_BASE (0xfe000000)
#define REG_ADDR_GPIO_BASE (REG_ADDR_BASE + 0x00200000)
#define REG_ADDR_GPIO_REG 0xa0
#define GPIO_PIN_BTN 23
#define GPIO_PIN_LED 24

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

static int mydriver_open(struct inode *inode, struct file *file){
	printk("mydriver_open");
	gpio_base = ioremap_nocache(REG_ADDR_GPIO_BASE, REG_ADDR_GPIO_REG);

	const u32 led = GPIO_PIN_LED;
	const u32 index = led/10;
	const u32 shift = (led%10)*3;
	const u32 mask = ~(0x7 << shift);
	gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);

	return 0;
}

static int mydriver_close(struct inode *inode, struct file *file){
	printk("mydriver_close");
	iounmap(gpio_base);
	return 0;
}

static ssize_t led_write(struct file *filp, const char *buf, size_t count, loff_t *pos){
	char c;
	int btn, i;
	printk("led_write");
	if(copy_from_user(&c, buf, sizeof(char)))
		return -EFAULT;
	printk(KERN_INFO "receive %c\n", c);
	switch(c){
		case 'A':
			gpio_direction_input(GPIO_PIN_BTN);
			btn = gpio_get_value(GPIO_PIN_BTN);
			printk("button = %d\n", btn);
			if(btn == 0){
				for(i = 0; i < 5; i++){
					gpio_base[7] = 1 << GPIO_PIN_LED;
					msleep(500);
					gpio_base[10] = 1 << GPIO_PIN_LED;
					msleep(500);
				}
				c = 'C';
			} else if(btn == 1) {
				for(i = 100; i > 1; i -= 5){
					gpio_base[7] = 1 << GPIO_PIN_LED;
					msleep(i);
					gpio_base[10] = 1 << GPIO_PIN_LED;
					msleep(i);
				}
				gpio_base[7] = 1 << GPIO_PIN_LED;
				msleep(1000);
				gpio_base[10] = 1 << GPIO_PIN_LED;
				c = 'C';
			}
			break;
		case 'B':
			for(i = 0; i < 10; i++){
				gpio_base[7] = 1 << GPIO_PIN_LED;
				msleep(200);
				gpio_base[10] = 1 << GPIO_PIN_LED;
				msleep(200);
			}
			c = 'C';
			break;
		case 'C':
			gpio_base[10] = 1 << GPIO_PIN_LED;
			break;
	}
	return 1;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t count, loff_t *pos){
	printk("led_read");
	int val = gpio_get_value(GPIO_PIN_BTN);
	put_user(val + '0', &buf[0]);
	return count;
}

static struct file_operations driver_fops = {
	.owner   = THIS_MODULE,
	.open    = mydriver_open,
	.release = mydriver_close,
	.write   = led_write,
	.read    = led_read
};

static int __init init_mod(void){
	int retval;
	retval = alloc_chrdev_region(&dev, 0, 1, "mydriver");
	if(retval < 0){
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
	printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__,MAJOR(dev));

	cdev_init(&cdv, &driver_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval < 0){
		printk(KERN_ERR "cdev_add failed. major:%d, minor:%d", MAJOR(dev), MINOR(dev));
		return retval;
	}

	cls = class_create(THIS_MODULE, "mydriver");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}

	device_create(cls, NULL, dev, NULL, "mydriver%d", MINOR(dev));

	return 0;
}

static void __exit cleanup_mod(void){
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
