#include <linux/input.h>	
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
/*libaries device file*/
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include "ili9255.h"
#include <linux/of_gpio.h>


#define ILI9255_OK 0

/*implementation struct ili9255 driver*/
struct ili9255_typedef {
    void *private_data;	
    struct class *class;
    struct device *device;
    struct cdev cdev;
    dev_t devid;
    int major;
    int minor;
    unsigned char buff_tx[2];
    struct spi_device *spi;
    struct device_node *nd; 
    int cmd_gpio;                                              
	int reset_gpio;		
};
struct ili9255_typedef ili9255;



int ili9255_open(struct inode * inode, struct file *file){
    printk("I'm in open\n");
    return 0;
}

int ili9255_release(struct inode *inode, struct file *file){
    printk("I'm in release");
    return 0;
}

ssize_t ili9255_read(struct file *file, char __user * buffer, size_t length, loff_t * offset){
    printk("I'm in read");
    return length;
}

ssize_t ili9255_write(struct file *file, const char __user * buffer, size_t length, loff_t *offset ){
    printk("I'm in write");
    return length;
}

/*implementation file operations*/
static struct file_operations ili9255_fops ={
    .owner= THIS_MODULE,
    .open = ili9255_open,
    .release = ili9255_release,
    .read = ili9255_read,
    .write = ili9255_write,
};

static int create_device_file_ili9255(void){
    if((alloc_chrdev_region(&ili9255.devid, 0, 1, "ili9255_dev")) <0){
    pr_err("Cannot allocate major number\n");
    return -1;
    }
    ili9255.major = MAJOR(ili9255.devid);
    ili9255.minor = MINOR(ili9255.devid);
    pr_info("Major = %d Minor = %d \n", ili9255.major, ili9255.minor);

    /*Creating cdev structure*/
    cdev_init(&ili9255.cdev,&ili9255_fops);

    /*Adding character device to the system*/
    if((cdev_add(&ili9255.cdev,ili9255.devid,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        unregister_chrdev_region(ili9255.devid,1);
        return -1;
    }
    /*Creating struct class*/
    if(IS_ERR(ili9255.class = class_create(THIS_MODULE,"ili9255_class"))){
        pr_err("Cannot create the struct class\n");
        unregister_chrdev_region(ili9255.devid,1);
        return -1;
    }
    /*Creating struct device*/
    if(IS_ERR(ili9255.device = device_create(ili9255.class,NULL,ili9255.devid,NULL,"ili9255_device"))){
        pr_err("Cannot create the Device 1\n");
        class_destroy(ili9255.class);
        unregister_chrdev_region(ili9255.devid,1);
        return -1;
    }
    return 0;
}


static int ili9255_probe(struct spi_device *spi)
{
    int ret;
    printk("I'm in probe\n");
    ret= create_device_file_ili9255();
    if(ret <0){
        pr_err("create_device_file_ili9255 failed\n");
    }
    /*impletment iio for spi driver*/
    spi->mode = SPI_MODE_0;
    spi->bits_per_word =8;
    ili9255.spi =spi;
    ili9255.nd = of_get_parent(spi->dev.of_node);

    if(ili9255.nd == NULL){
		printk("device node not find!\n");
		return -EINVAL;
	}
	ili9255.reset_gpio = of_get_named_gpio(ili9255.nd, "reset-gpio", 0);
	if(ili9255.reset_gpio < 0) {
		printk("can't get reset-gpio\n");
		return -EINVAL;
	}
	ili9255.cmd_gpio = of_get_named_gpio(ili9255.nd, "cmd-gpio", 0);
	if(ili9255.cmd_gpio < 0){
		printk("can't get cmdear_gpio\n");
		return -EINVAL;
    }
    printk("cmd_gpio : %d\n",ili9255.cmd_gpio);
    printk("reset_gpio : %d\n",ili9255.reset_gpio);
    gpio_direction_output(ili9255.cmd_gpio,GPIO_PIN_SET);
    gpio_direction_output(ili9255.reset_gpio,GPIO_PIN_SET);
    lcd_init(spi,ili9255.cmd_gpio,ili9255.reset_gpio);
    fill_rectangle(0, 0, WIDTH, HEIGHT, COLOR_WHITE);
    mdelay(500);
    draw_string(20, 50, COLOR_DARKVIOLET, 2, "Hello World");
    mdelay(500);
    return ret;
}

static int ili9255_remove(struct spi_device *spi)
{
    printk("I'm in remove\n");
    device_destroy(ili9255.class,ili9255.devid);
    class_destroy(ili9255.class);
    cdev_del(&ili9255.cdev);
    unregister_chrdev_region(ili9255.devid, 1);
	return ILI9255_OK;

}
static const struct of_device_id ili9255_dt_ids[] = {
	{ .compatible = "ti,ili9255", },
	{ }
};
MODULE_DEVICE_TABLE(of, ili9255_dt_ids);

static const struct spi_device_id ili9255_id[] = {
	{ .name = "ili9255", },
	{ }
};
MODULE_DEVICE_TABLE(spi, ili9255_id);

static struct spi_driver ili9255_driver = {
	.driver = {
		.name = "ili9255",
		.owner = THIS_MODULE,
		.of_match_table = ili9255_dt_ids,
	},
	.probe   = ili9255_probe,
	.remove  = ili9255_remove,
	.id_table	= ili9255_id,
};


int __init ili9255_init(void){
    int ret=0;
    ret = spi_register_driver(&ili9255_driver);
    printk("I'm in ili9255 init\n");
    return ret;
}

void __exit ili9255_exit(void){
    printk("I'm in exit\n");
    spi_unregister_driver(&ili9255_driver);
}

module_init(ili9255_init);
module_exit(ili9255_exit);
MODULE_AUTHOR("bui dinh hien");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple implementation of ili9255 spi driver");