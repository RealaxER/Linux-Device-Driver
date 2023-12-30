// SPDX-License-Identifier: GPL-2.0-only
/*
 * Characater driver simple, use struct class, dev, dev_t for implement device file
 *
 * Copyright 2023 VinaLinux.
 *
 * Author: Bùi Hiển (Mark) buihien29112002@gmail.com
 *
 */

#include <linux/module.h>    // all kernel modules
#include <linux/kernel.h>    // KERN_INFO
#include <linux/errno.h>     // EFAULT
#include <linux/device.h>    // device register
#include <linux/fs.h>        // file_operations
#include <linux/types.h>     // size_t
#include <linux/uaccess.h>   // copy_from/to_user


#define DEVICE1 "my_device1"
#define DEVICE2 "my_device2"
#define CLASS_NAME "my_class"
#define DEVICE_DEBUG "my_device"

static char msg[100] ={0};
static int major_device1;
static int major_device2;

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    ssize_t ret;
    
    if (length >= sizeof(msg))
        return -EINVAL;
    
    ret = copy_from_user(msg, buffer, length);
    if (ret)
        return -EFAULT;
    
    pr_info("Received message: %s\n", msg);
    
    return length;
}
int device_open(struct inode * _inode, struct file * file){
    pr_info("Device open \n");
    return 0;
}
int device_close(struct inode * _inode, struct file * file){
    pr_info("Device close \n");
    return 0;
}
static const struct file_operations __fops1 = {
    .owner = THIS_MODULE,
    .write = device_write,
    .open = device_open,
    .release = device_close,
};

static const struct file_operations __fops2 = {
    .owner = THIS_MODULE,
    .write = device_write,
    .open = device_open,
    .release = device_close,
};

static struct device *device1 = NULL;
static struct device *device2 = NULL;
static struct class *myclass = NULL;

static int hello_init(void)
{
    major_device1 = register_chrdev(0,DEVICE1,&__fops1);
    if(major_device1<0){
        pr_info("Not create file");
        return major_device1;
    }
    major_device2 = register_chrdev(0,DEVICE2,&__fops2);
    if(major_device2<0){
        pr_info("Not create file");
        return major_device2;
    }
    pr_info("Registed file with major number : %d\n",major_device1);
    pr_info("Registed file with major number : %d\n",major_device2);

    myclass = class_create(THIS_MODULE,CLASS_NAME);
    if(IS_ERR(myclass)){
        unregister_chrdev(major_device1,DEVICE1);
        pr_info("Dont register class \n");	
        return -1;
    }
    pr_info("Registered class \n");
    device1 = devic e_create(myclass,NULL,MKDEV(major_device1,0),NULL,DEVICE1);
    if(IS_ERR(device1)) {
        class_destroy(myclass);
        unregister_chrdev(major_device1,DEVICE1);
        pr_info("Don't register device1");
        return -1;
    }  
    device1 = device_create(myclass,NULL,MKDEV(major_device2,0),NULL,DEVICE2);
    if(IS_ERR(device2)) {
        class_destroy(myclass);
        unregister_chrdev(major_device2,DEVICE2);
        pr_info("Don't register device1");
        return -1;
    }   

    memset(msg,0,sizeof(msg));  
    printk(KERN_INFO DEVICE_DEBUG "Init!\n");
    return 0;
}

static void hello_exit(void)
{
    device_destroy(myclass,MKDEV(major_device1,0));
    device_destroy(myclass,MKDEV(major_device2,0));
    class_destroy(myclass);
    class_unregister(myclass);
    unregister_chrdev(major_device1,DEVICE1);
    printk("Goodbye!\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");

module_init(hello_init);
module_exit(hello_exit);
