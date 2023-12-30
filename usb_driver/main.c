/*
 * examplw Linux Usb Device Driver
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
 
#define USB_VENDOR_ID 0x10d6
#define USB_PRODUCT_ID 0x1101
 

int usb_register_dev(struct usb_interface *intf,
		     struct usb_class_driver *class_driver)
{
	int retval;
	int minor_base = class_driver->minor_base;
	int minor;
	char name[20];
 
#ifdef CONFIG_USB_DYNAMIC_MINORS
	/*
	 * We don't care what the device tries to start at, we want to start
	 * at zero to pack the devices into the smallest available space with
	 * no holes in the minor range.
	 */
	minor_base = 0;
#endif
 
	if (class_driver->fops == NULL)
		return -EINVAL;
	if (intf->minor >= 0)
		return -EADDRINUSE;
 
	mutex_lock(&init_usb_class_mutex);
	retval = init_usb_class();
	mutex_unlock(&init_usb_class_mutex);
 
	if (retval)
		return retval;
 
	dev_dbg(&intf->dev, "looking for a minor, starting at %d\n", minor_base);
 
	down_write(&minor_rwsem);
	for (minor = minor_base; minor < MAX_USB_MINORS; ++minor) {
		if (usb_minors[minor])
			continue;
 
		usb_minors[minor] = class_driver->fops;
		intf->minor = minor;
		break;
	}
	if (intf->minor < 0) {
		up_write(&minor_rwsem);
		return -EXFULL;
	}
 
	/* create a usb class device for this usb interface */
	snprintf(name, sizeof(name), class_driver->name, minor - minor_base);
	intf->usb_dev = device_create(usb_class->class, &intf->dev,
				      MKDEV(USB_MAJOR, minor), class_driver,
				      "%s", kbasename(name));
	if (IS_ERR(intf->usb_dev)) {
		usb_minors[minor] = NULL;
		intf->minor = -1;
		retval = PTR_ERR(intf->usb_dev);
	}
	up_write(&minor_rwsem);
	return retval;
}
EXPORT_SYMBOL_GPL(usb_register_dev);

static struct usb_device_id usb_drv_table[]={
	{
		USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID)
	},
	{}
};
static int __init usb_test_init(void)
{
	printk(KERN_INFO "Register the usb driver with the usb subsystem \n");
	return usb_register(&usb_drv_struct);
}
 
static void __exit usb_test_exit(void)
{
	printk(KERN_INFO "Deregister the usb driver with usb subsystem\n");
	usb_deregister(&usb_drv_struct);
}

static int usb_drv_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_host_interface *interface_desc;
	int ret;
 
	interface_desc = interface->cur_altsetting;
	printk(KERN_INFO "USB info %d now probed: (%04x:%04x)\n", interface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
	printk(KERN_INFO "ID->bNumEndpoints:%02x\n", interface_desc->desc.bNumEndpoints);
	printk(KERN_INFO "ID->bInterfaceClass:%02x\n", interface_desc->desc.bInterfaceClass);
 
	ret = usb_register_dev(interface,&usb_cd);
	if(ret)
	{
		printk(KERN_INFO "usb_register_dev erro: %d\n", ret);
	}
	else
	{
		printk(KERN_INFO "Minor number = %d\n", interface->minor);
	}
	return ret;
 
}
#define MYUSB_MINOR_BASE    250
static int myusb_open(struct inode *inode, struct file *file)
{
    pr_info("%s\n", __func__);
    return 0;
}
 
static int myusb_release(struct inode *inode, struct file *file)
{
    pr_info("%s\n", __func__);
    return 0;
}
 
static ssize_t myusb_write(struct file *file, const char *user_buffer,
                          size_t count, loff_t *ppos)
{
    pr_info("%s\n", __func__);
    return count;
}
 
static ssize_t myusb_read(struct file *file, char *buffer, size_t count,
                            loff_t *ppos)
{
    pr_info("%s\n", __func__);
    return 0;
}
 
static const struct file_operations myusb_fops = {
    .owner = THIS_MODULE,
    .open = myusb_open,
    .release = myusb_release,
    .read = myusb_read,
    .write = myusb_write,
};
 
static struct usb_class_driver usb_cd = {
    .name = "ActionsUSB%d",
    .fops = &myusb_fops,
    .minor_base = MYUSB_MINOR_BASE,
};
 
static void usb_drv_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "Disconneced and Release the MINOR number %d\n", interface->minor);
	usb_deregister_dev(interface, &usb_cd);
}
 
static struct usb_driver usb_drv_struct={
	.name = "Actions USB Driver",
	.probe = usb_drv_probe,
	.disconnect = usb_drv_disconnect,
	.id_table = usb_drv_table
};

 
MODULE_DEVICE_TABLE(usb, usb_drv_table);
module_init(usb_test_init);
module_exit(usb_test_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bui Dinh Hien");
MODULE_DESCRIPTION("USB test Driver");