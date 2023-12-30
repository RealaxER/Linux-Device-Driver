#ifndef __mylib__
#define __mylib__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

void my_test(void);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hien");
MODULE_DESCRIPTION("Linux My Lib");

#endif