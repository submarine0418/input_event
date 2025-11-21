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
	{ 0x5905b725, "tty_register_ldisc" },
	{ 0x92997ed8, "_printk" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xb5b54b34, "_raw_spin_unlock" },
	{ 0x41cc93f6, "input_event" },
	{ 0x32a0c53, "input_unregister_device" },
	{ 0x37a0cba, "kfree" },
	{ 0x83fc2de9, "tty_unregister_ldisc" },
	{ 0x4fc61f86, "kmalloc_caches" },
	{ 0x5443de3e, "__kmalloc_cache_noprof" },
	{ 0x99c44b73, "input_allocate_device" },
	{ 0xcab72f31, "input_register_device" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "33A90BDD7EDB76CFEB64364");
