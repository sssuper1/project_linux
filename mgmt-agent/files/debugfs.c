

#include "debugfs.h"

#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/sched.h> /* for linux/wait.h */
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/stddef.h>
#include <linux/stringify.h>
#include <linux/sysfs.h>
#include <net/net_namespace.h>

#include "mgmt_module.h"
#include "mgmt_netlink.h"
#define NODE_NUM 64

static struct dentry *mgmt_module_debugfs;

int mgmt_id_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%d\n", mgmt_info->id);
	return 0;
}

static int mgmt_module_id_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_id_print_text, NULL);
}

int mgmt_ip_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%d\n", mgmt_info->ip);
	return 0;
}

static int mgmt_module_ip_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_ip_print_text, NULL);
}

int mgmt_macaddr_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%s\n", mgmt_info->macaddr);
	return 0;
}

static int mgmt_module_macaddr_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_macaddr_print_text, NULL);
}

int mgmt_txrate_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%d\n", mgmt_info->txrate);
	return 0;
}

static int mgmt_module_txrate_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_txrate_print_text, NULL);
}

int mgmt_rxrate_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%d\n", mgmt_info->rxrate);
	return 0;
}

static int mgmt_module_rxrate_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_rxrate_print_text, NULL);
}

int mgmt_freq_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%d\n", mgmt_info->freq);
	return 0;
}

static int mgmt_module_freq_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_freq_print_text, NULL);
}

int mgmt_bw_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%d\n", mgmt_info->bw);
	return 0;
}

static int mgmt_module_bw_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_bw_print_text, NULL);
}

int mgmt_txpower_print_text(struct seq_file *seq, void *offset)
{
	seq_printf(seq, "%d\n", mgmt_info->txpower);
	return 0;
}

static int mgmt_module_txpower_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_txpower_print_text, NULL);
}



int mgmt_msg_print_text(struct seq_file *seq, void *offset)
{
	int i = 0;
//	seq_printf(seq,"节点号:%d\n",bat_jgk_node_msg.nodeid);
	seq_printf(seq,"收发包数  tx: %8d    rx: %8d\n",jgk_information_data_msg.traffic_queue_information.tx_inall//jgk_information_data_msg.traffic_queue_information.ogm_slot+jgk_information_data_msg.traffic_queue_information.ping_slot  \
//			+jgk_information_data_msg.traffic_queue_information.bcast_slot+jgk_information_data_msg.traffic_queue_information.ucast_slot
			,jgk_information_data_msg.traffic_queue_information.rx_outall);
	seq_puts(seq,"MAC:\n");
	seq_puts(seq,
		 "NODEID  JITTER  SNR    RSSI   MCS    GOOD   BAD    UCDS   NOISE\n");


		 
	for(i = 0; i < NODE_NUM; i ++){
			if(jgk_information_data_msg.mac_information_part2.snr[i] == 0 && jgk_information_data_msg.mac_information_part2.rssi[i]  == 0)
			{
			  jgk_information_data_msg.mac_information_part2.time_jitter[i] =0;
			  jgk_information_data_msg.mac_information_part2.mcs[i] = 0x0f;
			  jgk_information_data_msg.mac_information_part2.good[i] = 0;
			  jgk_information_data_msg.mac_information_part2.bad[i] = 0;
			  jgk_information_data_msg.mac_information_part2.ucds[i] = 0;
			  jgk_information_data_msg.mac_information_part2.noise[i] = 0;
			  
			}
		seq_printf(seq,"%4d   %4d   %4d   %4d   %4d   %4d   %4d   %4d   %4d\n",i,(short)jgk_information_data_msg.mac_information_part2.time_jitter[i] \
				,jgk_information_data_msg.mac_information_part2.snr[i],jgk_information_data_msg.mac_information_part2.rssi[i]  \
				,jgk_information_data_msg.mac_information_part2.mcs[i],jgk_information_data_msg.mac_information_part2.good[i]  \
				,jgk_information_data_msg.mac_information_part2.bad[i],jgk_information_data_msg.mac_information_part2.ucds[i]  \
				,jgk_information_data_msg.mac_information_part2.noise[i]);
	}
	return 0;
}

static int mgmt_module_msg_open(struct inode *inode, struct file *file)
{
	return single_open(file, mgmt_msg_print_text, NULL);
}

struct mgmt_module_debuginfo {
	struct attribute attr;
	const struct file_operations fops;
};



#define MGMT_MODULE_DEBUGINFO(_name, _mode, _open)		\
struct mgmt_module_debuginfo mgmt_module_debuginfo_##_name = {	\
	.attr = {					\
		.name = __stringify(_name),		\
		.mode = _mode,				\
	},						\
	.fops = {					\
		.owner = THIS_MODULE,			\
		.open = _open,				\
		.read	= seq_read,			\
		.llseek = seq_lseek,			\
		.release = single_release,		\
	},						\
}

static MGMT_MODULE_DEBUGINFO(mgmt_nodeid, 0444, mgmt_module_id_open);
static MGMT_MODULE_DEBUGINFO(mgmt_ip, 0444, mgmt_module_ip_open);
static MGMT_MODULE_DEBUGINFO(mgmt_macaddr, 0444, mgmt_module_macaddr_open);
static MGMT_MODULE_DEBUGINFO(mgmt_txrate, 0444, mgmt_module_txrate_open);
static MGMT_MODULE_DEBUGINFO(mgmt_rxrate, 0444, mgmt_module_rxrate_open);
static MGMT_MODULE_DEBUGINFO(mgmt_freq, 0444, mgmt_module_freq_open);
static MGMT_MODULE_DEBUGINFO(mgmt_bw, 0444, mgmt_module_bw_open);
static MGMT_MODULE_DEBUGINFO(mgmt_txpower, 0444, mgmt_module_txpower_open);
static MGMT_MODULE_DEBUGINFO(mgmt_msg, 0444, mgmt_module_msg_open);

static struct mgmt_module_debuginfo *mgmt_module_general_debuginfos[] = {
	&mgmt_module_debuginfo_mgmt_nodeid,
	&mgmt_module_debuginfo_mgmt_ip,
	&mgmt_module_debuginfo_mgmt_macaddr,
	&mgmt_module_debuginfo_mgmt_txrate,
	&mgmt_module_debuginfo_mgmt_rxrate,
	&mgmt_module_debuginfo_mgmt_freq,
	&mgmt_module_debuginfo_mgmt_bw,
	&mgmt_module_debuginfo_mgmt_txpower,
	&mgmt_module_debuginfo_mgmt_msg,
	NULL,
};

void mgmt_module_debugfs_init(void)
{
	struct mgmt_module_debuginfo **mgmt_debug;
	struct dentry *file;

	mgmt_module_debugfs = debugfs_create_dir(MGMT_MODULE_DEBUGFS_SUBDIR, NULL);
	if (mgmt_module_debugfs == ERR_PTR(-ENODEV))
		mgmt_module_debugfs = NULL;

	if (!mgmt_module_debugfs)
		goto err;

	for (mgmt_debug = mgmt_module_general_debuginfos; *mgmt_debug; ++mgmt_debug) {
		file = debugfs_create_file(((*mgmt_debug)->attr).name,
					   S_IFREG | ((*mgmt_debug)->attr).mode,
					   mgmt_module_debugfs, NULL,
					   &(*mgmt_debug)->fops);
		if (!file) {
			pr_err("Can't add general debugfs file: %s\n",
			       ((*mgmt_debug)->attr).name);
			goto err;
		}
	}

	return;
err:
	debugfs_remove_recursive(mgmt_module_debugfs);
	mgmt_module_debugfs = NULL;
}

void mgmt_module_debugfs_destroy(void)
{
	debugfs_remove_recursive(mgmt_module_debugfs);
	mgmt_module_debugfs = NULL;
}
