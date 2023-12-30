#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x2c111234, "module_layout" },
	{ 0xe8f30cce, "i2c_del_driver" },
	{ 0x76fc9e1b, "i2c_register_driver" },
	{ 0xec054ba9, "device_create" },
	{ 0x17ea0b17, "__class_create" },
	{ 0xeb2054f8, "cdev_add" },
	{ 0x94718d8d, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x93aedef8, "cdev_del" },
	{ 0x7d405946, "class_destroy" },
	{ 0x730cfd04, "device_destroy" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x86d1d142, "i2c_transfer" },
	{ 0xc5850110, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cti,mpu6050");
MODULE_ALIAS("of:N*T*Cti,mpu6050C*");
MODULE_ALIAS("i2c:mpu6050");
