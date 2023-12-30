#include <linux/input.h>	
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
/*libaries device file*/
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
/*industrial io for spi*/
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include "adxl.h"



#define ADXL345_OK 0

static void adxl345_test(void);

/*implementation struct adxl345 driver*/
struct adxl345_typedef {
    void *private_data;	
    struct class *class;
    struct device *device;
    struct cdev cdev;
    dev_t devid;
    int major;
    int minor;
    unsigned char buff_tx[2];
    struct spi_device *spi;
};
struct adxl345_typedef adxl345;



int adxl345_open(struct inode * inode, struct file *file){
    printk("I'm in open\n");
    adxl345_test();
    return 0;
}

int adxl345_release(struct inode *inode, struct file *file){
    printk("I'm in release");
    return 0;
}

ssize_t adxl345_read(struct file *file, char __user * buffer, size_t length, loff_t * offset){
    printk("I'm in read");
    return length;
}

ssize_t adxl345_write(struct file *file, const char __user * buffer, size_t length, loff_t *offset ){
    printk("I'm in write");
    return length;
}

/*implementation file operations*/
static struct file_operations adxl345_fops ={
    .owner= THIS_MODULE,
    .open = adxl345_open,
    .release = adxl345_release,
    .read = adxl345_read,
    .write = adxl345_write,
};

static int create_device_file_adxl345(void){
    if((alloc_chrdev_region(&adxl345.devid, 0, 1, "adxl345_dev")) <0){
    pr_err("Cannot allocate major number\n");
    return -1;
    }
    adxl345.major = MAJOR(adxl345.devid);
    adxl345.minor = MINOR(adxl345.devid);
    pr_info("Major = %d Minor = %d \n", adxl345.major, adxl345.minor);

    /*Creating cdev structure*/
    cdev_init(&adxl345.cdev,&adxl345_fops);

    /*Adding character device to the system*/
    if((cdev_add(&adxl345.cdev,adxl345.devid,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        unregister_chrdev_region(adxl345.devid,1);
        return -1;
    }
    /*Creating struct class*/
    if(IS_ERR(adxl345.class = class_create(THIS_MODULE,"adxl345_class"))){
        pr_err("Cannot create the struct class\n");
        unregister_chrdev_region(adxl345.devid,1);
        return -1;
    }
    /*Creating struct device*/
    if(IS_ERR(adxl345.device = device_create(adxl345.class,NULL,adxl345.devid,NULL,"adxl345_device"))){
        pr_err("Cannot create the Device 1\n");
        class_destroy(adxl345.class);
        unregister_chrdev_region(adxl345.devid,1);
        return -1;
    }
    return 0;
}


/*implement reg driver for spi*/
static int adxl345_write_regs(u8 reg, u8 *buf, int len) {
    int ret = 0;
    unsigned char *txdata;
    struct spi_message m;
    struct spi_transfer *t;
    /* 构建spi_transfer */
    t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if(!t) {
		return -ENOMEM;
	}
    txdata = kzalloc(sizeof(char)+len, GFP_KERNEL);
	if(!txdata) {
		kfree(txdata);				/* 释放内存 */
	}
    /* 第一步：发送要写入的寄存器地址 */
    txdata[0] = reg & 0XEF; //MOSI发送8位地址，但是实质地址只有前7位，第8位是读写位（W:0）
    memcpy(&txdata[1], buf, len);	/* 把len个寄存器拷贝到txdata里，等待发送 */
    t->tx_buf = txdata; //发送的数据
    t->len = 1 + len; //发送数据长度
    spi_message_init(&m); //初始化spi_message
    spi_message_add_tail(t,&m); //将spi_transfer添加进spi_message里面
    ret = spi_sync(adxl345.spi,&m);   //以同步方式发送数据
    if(ret < 0) {
        printk("spi_sync error!\r\n");
    }    
    kfree(txdata);				/* 释放内存 */
    kfree(t);					/* 释放内存 */
    return ret;
}

static int adxl345_read_regs(uint8_t reg, void *buf, int len) {
	int ret = -1;
	unsigned char txdata[1];
	unsigned char * rxdata;
	struct spi_message m;
	struct spi_transfer *t;
    
	t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);	/* 申请内存 */
	if(!t) {
		return -ENOMEM;
	}

	rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL);	/* 申请内存 */
	if(!rxdata) {
		goto out1;
	}
    memset(rxdata, 0, len);
	/* 一共发送len+1个字节的数据，第一个字节为
	寄存器首地址，一共要读取len个字节长度的数据，*/
	txdata[0] = (reg|0x80);		/* 写数据的时候首寄存器地址bit8要置1 */			
	t->tx_buf = txdata;			/* 要发送的数据 */
    t->rx_buf = rxdata;			/* 要读取的数据 */
	t->len = len;				/* t->len=发送的长度+读取的长度 */
	spi_message_init(&m);		/* 初始化spi_message */
	spi_message_add_tail(t, &m);/* 将spi_transfer添加到spi_message队列 */
	ret = spi_sync(adxl345.spi, &m);	/* 同步发送 */
	if(ret) {
		goto out2;
	}
	
    memcpy(buf, rxdata+1, len);  /* 只需要读取的数据 */

out2:
	kfree(rxdata);					/* 释放内存 */
out1:	
	kfree(t);						/* 释放内存 */
	
	return ret;
}

static uint8_t adxl345_read_onereg(uint8_t reg) {
    uint8_t data = 0;
    adxl345_read_regs(reg, &data, 1);
    return data;
}

/* 4.3.2 adxl345写一个寄存器 */
static void adxl345_write_onereg(uint8_t reg, uint8_t value) {
    adxl345_write_regs(reg, &value, 1);
}

/*implement spi driver*/
uint8_t adxl345_get_id(void)
{
	uint8_t result = 0;
	result = adxl345_read_onereg(DEVID);
	return result;
}


static int adxl345_reg_init(void)
{
    int ret =0;
    ret = adxl345_get_id();
	printk("ADXL345 Init ID:%d\n",ret);
	adxl345_write_onereg(INT_ENABLE, 0x00);
	adxl345_write_onereg(DATA_FORMAT, 0x0B);
	adxl345_write_onereg(BW_RATE, 0x1A);
	adxl345_write_onereg(POWER_CTL, 0x08);
	adxl345_write_onereg(INT_ENABLE, 0x14);
	if(ret <0){
        pr_err("Write reg fail, ret :%d\n",ret);
        return -1;
    }
    return ret;
}
static void adxl345_test(void){
    short asis_x;
	uint8_t x_low, x_high;

	x_low = adxl345_read_onereg(DATA_X0);
	x_high= adxl345_read_onereg(DATA_X1);

	asis_x = (short)(((uint16_t)x_high << 8) + x_low);
    printk("asis x : %d",asis_x);
}


static int adxl345_probe(struct spi_device *spi)
{
    int ret;
    printk("I'm in probe\n");
    ret= create_device_file_adxl345();
    if(ret <0){
        pr_err("create_device_file_adxl345 failed\n");
    }
    /*impletment iio for spi driver*/
    spi->mode = SPI_MODE_3;
    adxl345.spi =spi;
    ret = adxl345_reg_init();
    return ret;
}

static int adxl345_remove(struct spi_device *spi)
{
    printk("I'm in remove\n");
    device_destroy(adxl345.class,adxl345.devid);
    class_destroy(adxl345.class);
    cdev_del(&adxl345.cdev);
    unregister_chrdev_region(adxl345.devid, 1);
	return ADXL345_OK;

}
static const struct of_device_id adxl345_dt_ids[] = {
	{ .compatible = "ti,adxl345", },
	{ }
};
MODULE_DEVICE_TABLE(of, adxl345_dt_ids);

static const struct spi_device_id adxl345_id[] = {
	{ .name = "adxl345", },
	{ }
};
MODULE_DEVICE_TABLE(spi, adxl345_id);

static struct spi_driver adxl345_driver = {
	.driver = {
		.name = "adxl345",
		.owner = THIS_MODULE,
		.of_match_table = adxl345_dt_ids,
	},
	.probe   = adxl345_probe,
	.remove  = adxl345_remove,
	.id_table	= adxl345_id,
};


int __init adxl345_init(void){
    int ret=0;
    ret = spi_register_driver(&adxl345_driver);
    printk("I'm in adxl345 init\n");
    return ret;
}

void __exit adxl345_exit(void){
    printk("I'm in exit\n");
    spi_unregister_driver(&adxl345_driver);
}

module_init(adxl345_init);
module_exit(adxl345_exit);
MODULE_AUTHOR("bui dinh hien");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple implementation of adxl345 spi driver");