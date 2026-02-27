#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "mgmt_module.h"
#include "mgmt_netlink.h"
#include "debugfs.h"

Smgmt_transmit_info* mgmt_info;

static int __init mgmt_module_init(void)
{
	mgmt_info = kzalloc(sizeof(Smgmt_transmit_info),GFP_KERNEL);
	mgmt_module_debugfs_init();
//	printk("mgmt_module_init start\n");
	mgmt_netlink_register();
	return 0;
}

static void __exit mgmt_module_exit(void)
{
	mgmt_module_debugfs_destroy();
	mgmt_netlink_unregister();
	kfree(mgmt_info);
}

module_init(mgmt_module_init);
module_exit(mgmt_module_exit);

MODULE_LICENSE("GPL");
