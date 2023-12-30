#include <linux/input.h>	
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
/*libaries device file*/
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include "st7789.h"
#include <linux/of_gpio.h>


#define ST7789_OK 0

/*implementation struct st7789 driver*/
struct st7789_typedef {
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
struct st7789_typedef st7789;



int st7789_open(struct inode * inode, struct file *file){
    printk("I'm in open\n");
    return 0;
}

int st7789_release(struct inode *inode, struct file *file){
    printk("I'm in release");
    return 0;
}

ssize_t st7789_read(struct file *file, char __user * buffer, size_t length, loff_t * offset){
    printk("I'm in read");
    return length;
}

ssize_t st7789_write(struct file *file, const char __user * buffer, size_t length, loff_t *offset ){
    printk("I'm in write");
    return length;
}

/*implementation file operations*/
static struct file_operations st7789_fops ={
    .owner= THIS_MODULE,
    .open = st7789_open,
    .release = st7789_release,
    .read = st7789_read,
    .write = st7789_write,
};

static int create_device_file_st7789(void){
    if((alloc_chrdev_region(&st7789.devid, 0, 1, "st7789_dev")) <0){
    pr_err("Cannot allocate major number\n");
    return -1;
    }
    st7789.major = MAJOR(st7789.devid);
    st7789.minor = MINOR(st7789.devid);
    pr_info("Major = %d Minor = %d \n", st7789.major, st7789.minor);

    /*Creating cdev structure*/
    cdev_init(&st7789.cdev,&st7789_fops);

    /*Adding character device to the system*/
    if((cdev_add(&st7789.cdev,st7789.devid,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        unregister_chrdev_region(st7789.devid,1);
        return -1;
    }
    /*Creating struct class*/
    if(IS_ERR(st7789.class = class_create(THIS_MODULE,"st7789_class"))){
        pr_err("Cannot create the struct class\n");
        unregister_chrdev_region(st7789.devid,1);
        return -1;
    }
    /*Creating struct device*/
    if(IS_ERR(st7789.device = device_create(st7789.class,NULL,st7789.devid,NULL,"st7789_device"))){
        pr_err("Cannot create the Device 1\n");
        class_destroy(st7789.class);
        unregister_chrdev_region(st7789.devid,1);
        return -1;
    }
    return 0;
}


static int st7789_probe(struct spi_device *spi)
{
    int ret;
    printk("I'm in probe\n");
    ret= create_device_file_st7789();
    if(ret <0){
        pr_err("create_device_file_st7789 failed\n");
    }
    /*impletment iio for spi driver*/
    spi->mode = SPI_MODE_0;
    spi->bits_per_word =8;
    st7789.spi =spi;
    st7789.nd = of_get_parent(spi->dev.of_node);

    if(st7789.nd == NULL){
		printk("device node not find!\n");
		return -EINVAL;
	}
	st7789.reset_gpio = of_get_named_gpio(st7789.nd, "reset-gpio", 0);
	if(st7789.reset_gpio < 0) {
		printk("can't get reset-gpio\n");
		return -EINVAL;
	}
	st7789.cmd_gpio = of_get_named_gpio(st7789.nd, "cmd-gpio", 0);
	if(st7789.cmd_gpio < 0){
		printk("can't get cmdear_gpio\n");
		return -EINVAL;
    }
    printk("cmd_gpio : %d\n",st7789.cmd_gpio);
    printk("reset_gpio : %d\n",st7789.reset_gpio);
    gpio_direction_output(st7789.cmd_gpio,GPIO_PIN_SET);
    gpio_direction_output(st7789.reset_gpio,GPIO_PIN_SET);
    ST7789_Init(spi,st7789.cmd_gpio,st7789.reset_gpio);
    return ret;
}

static int st7789_remove(struct spi_device *spi)
{
    printk("I'm in remove\n");
    device_destroy(st7789.class,st7789.devid);
    class_destroy(st7789.class);
    cdev_del(&st7789.cdev);
    unregister_chrdev_region(st7789.devid, 1);
	return ST7789_OK;

}
static const struct of_device_id st7789_dt_ids[] = {
	{ .compatible = "ti,st7789", },
	{ }
};
MODULE_DEVICE_TABLE(of, st7789_dt_ids);

static const struct spi_device_id st7789_id[] = {
	{ .name = "st7789", },
	{ }
};
MODULE_DEVICE_TABLE(spi, st7789_id);

static struct spi_driver st7789_driver = {
	.driver = {
		.name = "st7789",
		.owner = THIS_MODULE,
		.of_match_table = st7789_dt_ids,
	},
	.probe   = st7789_probe,
	.remove  = st7789_remove,
	.id_table	= st7789_id,
};


int __init st7789_init(void){
    int ret=0;
    ret = spi_register_driver(&st7789_driver);
    printk("I'm in st7789 init\n");
    return ret;
}

void __exit st7789_exit(void){
    printk("I'm in exit\n");
    spi_unregister_driver(&st7789_driver);
}

module_init(st7789_init);
module_exit(st7789_exit);
MODULE_AUTHOR("bui dinh hien");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple implementation of st7789 spi driver");