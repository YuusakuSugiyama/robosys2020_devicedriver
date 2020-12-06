#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/io.h>

MODULE_AUTHOR("Ryuichi Ueda, Jitsukawa Hikaru");
MODULE_DESCRIPTION("driver for control LED with button")
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

#define GPIO_PIN_BTN 23
#define GPIO_PIN_LED 24
#define REG_ADDR_BASE (0xfe000000)
#define REG_ADDR_GPIO_BASE (REG_ADDR_BASE + 0x00200000)
#define REG_ADDR_GPIO_REG (0x00a0)

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
	int btn;
	printk("led_write");
	if(copy_from_user(&c, buf, sizeof(char)))
		return -EFAULT;
	printk(KERN_INFO "receive %c", c);
	switch(c){
		case 'A':
			gpio_direction_input(GPIO_PIN_BTN);
			btn = gpio_get_value(GPIO_PIN_BTN);
			printk("button = %d", btn);
			if(btn = 0) gpio_base[7] = 1 << GPIO_PIN_LED;
			else if(btn = 1) gpio_base[10] = 1 << GPIO_PIN_LED;
			break;
		case 'B':
			gpio_base[7] = 1 << GPIO_PIN_LED;
			break;
	}
	return 1;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	printk("led_read");
	int val = gpio_get_value(GPIO_PIN_LED);
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
	printk(KERN_ERR "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));

	cdev_init(&cdv, &driver_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval < 0){
		printk(KERN_ERR "cdev_add failed. major:%d", MAJOR(dev), MINOR(dev));
		return retval;
	}

	cls = class_create(THIS_MODULE, "mydriver");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	device_create(cls, NULL, dev, NULL, "myled%d", MINOR(dev));

	return 0;
}

static void __exit cleanup_mod(void){
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n",__FILE__,MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
