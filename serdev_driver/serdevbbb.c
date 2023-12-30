#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/of_device.h>
#include <linux/device.h>
#include <linux/uaccess.h>   // copy_from/to_userm
#include <linux/fs.h>  
#include <linux/of.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bui Dinh Hien");
MODULE_DESCRIPTION("Device driver uart interface device file");


/* Declate the probe and remove functions */
static int serdev_echo_probe(struct serdev_device *serdev);
static void serdev_echo_remove(struct serdev_device *serdev);
void serdev_write(const char *buffer, size_t length);

static int file_open (struct inode * inode, struct file *file);
static int file_release (struct inode * inode, struct file *file);
static ssize_t file_write (struct file * file, const char __user * buff, size_t length, loff_t * loff_of);
static ssize_t file_read (struct file * file,char __user * buff, size_t length, loff_t * loff_of);


static int serdev_receiver(struct serdev_device *serdev, const unsigned char *buffer, size_t size);
void serdev_write(const char *buffer, size_t length);
static void serdev_write_callback (struct serdev_device * serdev);



static struct of_device_id serdev_echo_ids[] = {
	{
		.compatible = "hc,hc05",
	}, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, serdev_echo_ids);

static struct serdev_device_driver serdev_echo_driver = {
	.probe = serdev_echo_probe,
	.remove = serdev_echo_remove,
	.driver = {
		.name = "serdev_driver",
		.of_match_table = serdev_echo_ids,
	},
};

struct file_operations hc05_fops={
	.owner      = THIS_MODULE,
    .read       = file_read,
    .write      = file_write,
    .open       = file_open,
    .release    = file_release,
};
struct uart4_driver{
	struct class *class;
	struct device *device;
	dev_t dev;
	char write[255];
	char read[255];
	struct cdev cdev;
	struct device* container;
};
struct uart4_driver hc05;

/**
 * @brief Function for file operations
*/
static int file_open (struct inode * inode, struct file *file){
	printk("/dev/hc05 opened\n");
	return 0;
}	
static int file_release (struct inode * inode, struct file *file){
	printk("/dev/hc05 released\n");
	return 0;
}
static ssize_t file_write (struct file * file, const char __user * buff, size_t length, loff_t * loff_of){
	/*Copy data from user to buffer write hc05*/
	int ret;
	memset(hc05.write, 0, 255);
	ret = copy_from_user(hc05.write,buff,length);
	/*it return 0 if copy successfully*/
	if(ret){	
		printk("/dev/hc05 copy don't succeed\n");
		return -1;
	}
	serdev_write(hc05.write,length);
	return length;
}
static ssize_t file_read (struct file * file,char __user * buff, size_t length, loff_t * loff_of){
	/*copy data from kernel to user*/
	size_t length_read = strlen(hc05.read);	
	if(copy_to_user(buff,hc05.read,length_read)){
		printk("receiver error\n");
	}
	return length_read;
}

/**
 * @brief Callback is called whenever a character is received
 */
static const struct serdev_device_ops serdev_echo_ops = {
	.receive_buf = serdev_receiver,
	.write_wakeup = serdev_write_callback,
};

static void serdev_write_callback (struct serdev_device * serdev){}
static int serdev_receiver(struct serdev_device *serdev, const unsigned char *buffer, size_t size) {
	memset(hc05.read, 0, 255);
	strncpy(hc05.read,buffer,size);
	printk("serdev_echo - Received %d bytes with \"%s\"\n", size, hc05.read);
    return size;
}
void serdev_write(const char *buffer, size_t length){
	int status;
	struct serdev_device *my_serdev = container_of(hc05.container,struct serdev_device,dev);
	serdev_write_callback(my_serdev);
	status = serdev_device_write_buf(my_serdev, buffer,length);
	printk("serdev_echo - Wrote %d bytes.\n", status);
}	

/**
 * @brief This function is called on loading the driver 
 */
static int serdev_echo_probe(struct serdev_device *serdev) {
	int status;
	/*notice that i'm in probe serdev*/
	printk("serdev_echo - Now I am in the probe function!\n");
	serdev_device_set_client_ops(serdev, &serdev_echo_ops);
	/*check serdev can open successfully ? */
	status = serdev_device_open(serdev);
	if(status) {
		printk("serdev_echo - Error opening serial port!\n");
		return -status;
	}
	/*Register device file and class*/
#define HC05_FILE "file_hc05"
	if((alloc_chrdev_region(&hc05.dev, 0, 1,HC05_FILE )) <0){
			pr_err("Cannot allocate major number\n");
			return -1;
	}
	cdev_init(&hc05.cdev,&hc05_fops);

	/*add file for cdev manage*/
#define COUNT_FILE_ADD 1
	cdev_add(&hc05.cdev,hc05.dev,COUNT_FILE_ADD);

	/*create class manager device hc05*/
#define HC05_CLASS "class_hc05"
	hc05.class = class_create(THIS_MODULE,HC05_CLASS);
	if(IS_ERR(hc05.class)){
        printk("class not created\n");
        return -ENOMEM;
    }

	/*create file manager device file hc05*/
#define HC05_DEVICE "device_hc05"
	hc05.device = device_create(hc05.class,NULL,hc05.dev,NULL,HC05_DEVICE);

	/*Configuration uart*/
	printk("serdev_echo -registed\n");
	serdev_device_set_baudrate(serdev, 9600);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);
	hc05.container = &serdev->dev;
	printk("Configuration uart successfully\n");
	return 0;
}

/**
 * @brief This function is called on unloading the driver 
 */
static void serdev_echo_remove(struct serdev_device *serdev) {
	printk("serdev_echo - Now I am in the remove function\n");
	pm_runtime_disable(&serdev->dev);
	printk("pm runtime disable\n");
	cdev_del(&hc05.cdev);
	printk("del\n");
    device_destroy(hc05.class,hc05.dev);
	printk("device destroy\n");
	class_destroy(hc05.class);
	printk("class destroy\n");
    unregister_chrdev_region(hc05.dev, 1);
	printk("cdev unregister\n");
}
/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("serdev_echo - Loading the driver...\n");
	if(serdev_device_driver_register(&serdev_echo_driver)) {
		printk("serdev_echo - Error! Could not load driver\n");
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void my_exit(void) {

	printk("serdev_echo - Unload driver");
	serdev_device_driver_unregister(&serdev_echo_driver);
}

module_init(my_init);
module_exit(my_exit);

