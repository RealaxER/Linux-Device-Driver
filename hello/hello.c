#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include "mylib.h"
int __init hello_init(void)
{
	printk("BuiHien Hello world 1.\n");
	my_test();
	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

void __exit hello_exit(void)
{
	printk("BuiHien Goodbye world 1.\n");
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World kernel module");


