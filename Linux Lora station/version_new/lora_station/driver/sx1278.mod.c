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
	{ 0xafbeba1c, "__spi_register_driver" },
	{ 0xe2964344, "__wake_up" },
	{ 0x1a283f39, "_dev_info" },
	{ 0xd51bf3d7, "driver_unregister" },
	{ 0xdcb764ad, "memset" },
	{ 0x9c771906, "spi_sync" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xe095e43a, "device_destroy" },
	{ 0x4a41ecb3, "class_destroy" },
	{ 0x607587f4, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x64898fd0, "spi_write_then_read" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x1000e51, "schedule" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0xfb077e23, "_dev_warn" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0x41e0584a, "spi_setup" },
	{ 0xfa61d21, "devm_kmalloc" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x832481e, "devm_gpiod_get" },
	{ 0xa6f50b3, "gpiod_to_irq" },
	{ 0x11437cf9, "devm_request_threaded_irq" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x122c3a7e, "_printk" },
	{ 0xcb944b71, "gpiod_set_value" },
	{ 0xf9a482f9, "msleep" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x5d9d9fd4, "cdev_init" },
	{ 0xcc335c1c, "cdev_add" },
	{ 0xf311fc60, "class_create" },
	{ 0x93ab9e33, "device_create" },
	{ 0xca311bb8, "dev_err_probe" },
	{ 0xf810f451, "_dev_err" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x39ff040a, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:sx1278");
MODULE_ALIAS("of:N*T*Csemtech,sx1278");
MODULE_ALIAS("of:N*T*Csemtech,sx1278C*");

MODULE_INFO(srcversion, "4D91A13E26145F6E941CA77");
