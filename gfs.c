#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

int __init initFS(void)
{
    printk(KERN_INFO "Module is installed!\n");
    return 0;
}

void __exit exitFS(void)
{
    printk(KERN_INFO "Module is removed!\n");
}

module_init(initFS);
module_exit(exitFS);

MODULE_AUTHOR("Gordiig");
MODULE_LICENSE("GPL");