#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xaee657ee, "__class_create" },
	{ 0xf2d7865d, "device_create" },
	{ 0xfd205a2d, "cdev_init" },
	{ 0x3fd2f8f8, "cdev_add" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x92997ed8, "_printk" },
	{ 0x5708ebf2, "cdev_del" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "D22B6296006EFD19485E9C5");
