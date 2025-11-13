#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

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
	{ 0x122c3a7e, "_printk" },
	{ 0x453c8403, "pci_msi_enabled" },
	{ 0xe92d0c3e, "pci_iounmap" },
	{ 0xf92d6d0f, "pci_release_region" },
	{ 0xdac1fff4, "pci_disable_device" },
	{ 0x5d72f991, "pci_disable_msi" },
	{ 0xa78af5f3, "ioread32" },
	{ 0xfd3de8a2, "pci_unregister_driver" },
	{ 0xd6abca80, "pci_enable_device_mem" },
	{ 0x28917712, "pci_set_master" },
	{ 0x67a33343, "devm_kmalloc" },
	{ 0x18039951, "pci_request_region" },
	{ 0x83e0a4a9, "pci_iomap" },
	{ 0x6a8cca5a, "pci_enable_msi" },
	{ 0x3b4cf56c, "_dev_info" },
	{ 0x427de137, "devm_request_threaded_irq" },
	{ 0x8792ad33, "_dev_warn" },
	{ 0x3a1b27aa, "_dev_err" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x8b091dd, "__pci_register_driver" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xc6227e48, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("pci:v00001234d000011E8sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "6417D6401C0DBC8C0F35EE6");
