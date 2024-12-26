#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Meloy");
MODULE_DESCRIPTION("A module with a /proc entry for TSU");

#define PROC_FILE_NAME "tsulab"

static int proc_show(struct seq_file *m, void *v) {
    unsigned int cpu;
    seq_printf(m, "List of CPU cores:\n");

    for_each_online_cpu(cpu) {
        seq_printf(m, "CPU core: %u\n", cpu);
    }

    return 0;
}

static int proc_open(struct inode *inode, struct file *file) {
    return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_file_ops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init my_module_init(void) {
    struct proc_dir_entry *entry;

    printk(KERN_INFO "Welcome to the Tomsk State University\n");

    entry = proc_create(PROC_FILE_NAME, 0, NULL, &proc_file_ops);
    if (!entry) {
        printk(KERN_ERR "Failed to create /proc/%s\n", PROC_FILE_NAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "/proc/%s created\n", PROC_FILE_NAME);
    return 0;
}

static void __exit my_module_exit(void) {
    remove_proc_entry(PROC_FILE_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_FILE_NAME);
    printk(KERN_INFO "Tomsk State University forever!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);