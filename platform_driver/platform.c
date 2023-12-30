// SPDX-License-Identifier: GPL-2.0-only
/*
 * Characater driver simple, use struct class, dev, dev_t for implement device file
 *
 * Copyright 2023 VinaLinux.
 *
 * Author: Bùi Hiển (Mark) buihien29112002@gmail.com
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fs.h>        // file_operations
#include <linux/uaccess.h>   // copy_from/to_user
#include <linux/device.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>   // copy_from/to_userm

#define TEST_FILE "my_file"
#define TEST_CLASS "my_class"
#define TEST_DEVICE "my_device"

MODULE_AUTHOR("bui dinh hien");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform driver simple");


// nguyên maãu hàm 
int file_open(struct inode * _inode, struct file *_file);
int file_release(struct inode * _inode, struct file *_file);
int test_probe(struct platform_device * test_device);
int test_remove(struct platform_device *test_device);
ssize_t file_write(struct file * _file, const char __user * _buff, size_t _size, loff_t * _loff_t);

struct _test_driver{
    struct cdev _cdev;
    dev_t _dev;
    struct class *_class;
    struct device *_device;
    char msg[100];
};
struct _test_driver test_driver;

static struct file_operations __fops = {
    .owner = THIS_MODULE, //
    .open = file_open,
    .release = file_release,
    .write = file_write,
};
static const struct of_device_id test_of_match[] = {
    { .compatible = "ti,mpu6050"},
    {}
};
static struct platform_driver platform_test_driver  = {
    .driver = {
        .name = "platform",
        .of_match_table = test_of_match,
    },
    .probe = test_probe,
    .remove =test_remove,
};

int platform_init(void){
    int ret;
    printk("platform_init\n");
    ret = platform_driver_register(&platform_test_driver);
    printk("ret : %d\n",ret);
    memset(test_driver.msg,0,sizeof(test_driver.msg));  
    return 0;
}

void platform_exit(void){
    printk("platform_exit\n");
    platform_driver_unregister(&platform_test_driver);
}
int file_open(struct inode * _inode, struct file *_file){
    printk("i'm in the open function\n");
    return 0;
}
int file_release(struct inode * _inode, struct file *_file){
    printk("i'm in the release function\n");
    return 0;
}
ssize_t file_write(struct file * _file, const char __user * _buff, size_t _size, loff_t * _loff_t){
    ssize_t ret;
    ret = copy_from_user(test_driver.msg,_buff,_size);
    if(ret){
        printk("copy denied\n");
        return -1;
    }
    pr_info("Received message: %s\n",test_driver.msg);
    return _size;
}

int test_probe(struct platform_device * test_device){
    printk("test probe function\n");
    // đăng kí major và minor cho file một cách tự động 
    alloc_chrdev_region(&test_driver._dev,0,1,TEST_FILE);
    // tự chọn major register_chrdev_region();
    cdev_init(&test_driver._cdev, &__fops); 
    // liên kết với cdev và file 
    // add device vào list cdev với số lượng là 1 device file
    cdev_add(&test_driver._cdev, test_driver._dev,1);

    test_driver._class = class_create(THIS_MODULE,TEST_CLASS);
    if(IS_ERR(test_driver._class)){
        printk("_class not created\n");
        return -ENOMEM;
    }
    test_driver._device = device_create(test_driver._class , NULL,test_driver._dev , NULL , TEST_DEVICE);
    return 0;
}
int test_remove(struct platform_device *test_device){
    printk("test remove function\n");
    cdev_del(&test_driver._cdev);
    device_destroy(test_driver._class,test_driver._dev);
    class_destroy(test_driver._class);
    class_unregister(test_driver._class);
    unregister_chrdev_region(test_driver._dev, 1);

    return 0;
}
module_init(platform_init);
module_exit(platform_exit);
