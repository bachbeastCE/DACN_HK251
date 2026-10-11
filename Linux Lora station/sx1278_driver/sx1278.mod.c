#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x73ce0416, "__spi_register_driver" },
	{ 0xe2964344, "__wake_up" },
	{ 0x914217ee, "_dev_info" },
	{ 0xd51bf3d7, "driver_unregister" },
	{ 0xdcb764ad, "memset" },
	{ 0x77afeaad, "spi_sync" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x7d958a42, "device_destroy" },
	{ 0x4a41ecb3, "class_destroy" },
	{ 0x607587f4, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x58d43c78, "spi_write_then_read" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x1000e51, "schedule" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x706d56f4, "_dev_warn" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0xaafa1bec, "spi_setup" },
	{ 0xfa61d21, "devm_kmalloc" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x832481e, "devm_gpiod_get" },
	{ 0xb5e73770, "gpiod_to_irq" },
	{ 0x11437cf9, "devm_request_threaded_irq" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x122c3a7e, "_printk" },
	{ 0x9115abcf, "gpiod_set_value" },
	{ 0xf9a482f9, "msleep" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x5d9d9fd4, "cdev_init" },
	{ 0xcc335c1c, "cdev_add" },
	{ 0xf311fc60, "class_create" },
	{ 0xab5c9881, "device_create" },
	{ 0x8d40b0ec, "dev_err_probe" },
	{ 0x737adc86, "_dev_err" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x39ff040a, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:sx1278");
MODULE_ALIAS("of:N*T*Csemtech,sx1278");
MODULE_ALIAS("of:N*T*Csemtech,sx1278C*");

MODULE_INFO(srcversion, "9223A66633C78ABC3B33132");
