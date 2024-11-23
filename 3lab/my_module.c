#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Meloy");
MODULE_DESCRIPTION("A simple module for Tomsk State University");

static int __init my_module_init(void) {
    printk(KERN_INFO "Welcome to the Tomsk State University\n");
    return 0;
}

static void __exit my_module_exit(void) {
    printk(KERN_INFO "Tomsk State University forever!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);