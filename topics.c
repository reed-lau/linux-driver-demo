#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>

#define TOPICS_MAJOR 231
#define DEVICE_NAME "TopicsModule"

static int i = 0;

static int topics_open(struct inode *inode, struct file *file){
    printk(KERN_EMERG "topics open.\n");
    return 0;
}

static ssize_t topics_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos){
    printk(KERN_EMERG "topics write. i = %d\n", ++i);
    return 0;
}

static struct file_operations topics_flops = {
    .owner  =   THIS_MODULE,
    .open   =   topics_open,
    .write  =   topics_write,
};

static int __init topics_init(void){
    int ret;

    ret = register_chrdev(TOPICS_MAJOR,DEVICE_NAME, &topics_flops);
    if (ret < 0) {
        printk(KERN_EMERG DEVICE_NAME " can't register major number.\n");
        return ret;
    }
    printk(KERN_EMERG DEVICE_NAME " initialized.\n");
    return 0;
}

static void __exit topics_exit(void){
    unregister_chrdev(TOPICS_MAJOR, DEVICE_NAME);
    printk(KERN_EMERG DEVICE_NAME " removed.\n");
}

module_init(topics_init);
module_exit(topics_exit);
MODULE_LICENSE("GPL");
