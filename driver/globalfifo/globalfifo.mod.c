#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x355992de, "module_layout" },
	{ 0x5e43d4a1, "cdev_del" },
	{ 0x126bda94, "per_cpu__current_task" },
	{ 0xb6d47bc4, "kmalloc_caches" },
	{ 0xd86b4bb5, "cdev_init" },
	{ 0x6980fe91, "param_get_int" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0x71356fba, "remove_wait_queue" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0x45d11c43, "down_interruptible" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xff964b25, "param_set_int" },
	{ 0xffc7c184, "__init_waitqueue_head" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0xb72397d5, "printk" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0x748caf40, "down" },
	{ 0x6e18b7ea, "cdev_add" },
	{ 0xde4e44c2, "kmem_cache_alloc" },
	{ 0x4292364c, "schedule" },
	{ 0x642e54ac, "__wake_up" },
	{ 0x650fb346, "add_wait_queue" },
	{ 0x37a0cba, "kfree" },
	{ 0x3f1899f1, "up" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "58C5075354866E70592F0A4");
