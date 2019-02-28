#include <linux/anon_inodes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/falloc.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/shmem_fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include "topics.h"

#define TOPICS_MAJOR 231
#define DEVICE_NAME "topics"

static int i = 0;
struct file *anon_file = NULL;
struct file *filp = NULL;

struct topics {
    char name[1024];
    struct list_head topics_list;
    struct file *file;
    size_t size;
};

static DEFINE_MUTEX(topics_mutex);

static struct kmem_cache *topics_cache __read_mostly;


static int topics_open(struct inode *inode, struct file *file){
    struct topics *topic;
    int ret;

    ret = generic_file_open(inode, file);
    if (ret)
        return ret;

    topic = kmem_cache_zalloc(topics_cache, GFP_KERNEL);
    if ( !topic )
        return -ENOMEM;

    INIT_LIST_HEAD(&topic->topics_list);
    memcpy(topic->name, "topic", 5);
    file->private_data = topic;

    printk(KERN_EMERG "topics: open\n");
    return 0;
}

static int topics_release(struct inode *inode, struct file *file) {
    struct topics *topic = file->private_data;

    printk(KERN_EMERG "topics: close total\n");
    if ( topic->file )
        fput(topic->file);

    kmem_cache_free(topics_cache, topic);

    return 0;
}

static int set_name(struct topics *topic, void __user *name) {
    printk(KERN_EMERG "topics: set_name enter topic=%p name=%p\n", topic, name);
    int ret = 0;
    char local_name[1024];

    strncpy_from_user(local_name, name, 1024);

    mutex_lock(&topics_mutex);
    if ( topic->file )
        ret = -EINVAL;
    else
        strcpy(topic->name, local_name);
    mutex_unlock(&topics_mutex);

    return ret;
}

static int get_fd(struct topics *topic) {
    int ret = 0;
    printk(KERN_EMERG "topics: get_fd topic=%p\n", topic);

    if ( IS_ERR(filp) ) {
    // if ( !topic->file ) {
        filp = filp_open("liuwei", O_CREAT | O_RDWR | O_CLOEXEC, 0666);
        if ( IS_ERR(filp) )
            ret = -EINVAL;

        topic->file = filp; 
    } else {
        topic->file = filp;
        get_file(filp);
    }

    int fd = get_unused_fd_flags(0);

    fd_install(fd, topic->file);
    ret = fd;

    get_file(topic->file);

    return ret;
}

static int xx_release(struct inode *inode, struct file *file) {
    printk(KERN_EMERG "topics: ------------------------------------------ release\n");
    fput(anon_file);
    anon_file = NULL;
    return 0;
}

static int xx_mmap(struct file *file, struct vmarea_struct *vma) {
    printk(KERN_EMERG "topics: mmap\n");
    return 0;
}

struct file_operations xx_fops ={
    .release = xx_release,
};

static long topics_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct topics *topic = file->private_data;
    int ret = -ENOTTY;

    switch (cmd) {
        case TOPICS_SET_NAME:
            ret = set_name(topic, (void __user*)arg);
            break;
        case TOPICS_GET_NAME:
            break;
        case TOPICS_SET_SIZE:
            break;
        case TOPICS_GET_SIZE:
            break;
        case TOPICS_GET_FD:

            if ( !anon_file ) {
                anon_file = shmem_file_setup("[liuwei]", 1000, 0);
                // anon_file = anon_inode_getfile("[liuwei]", &xx_fops, NULL, O_CREAT | O_RDWR | O_CLOEXEC);
            } else {
                get_file(anon_file);
            }

            int fd = get_unused_fd_flags(O_CLOEXEC);
            fd_install(fd, anon_file);
            ret = fd;

            printk(KERN_EMERG "topics: process:%lu\n", anon_file->f_count);
            break;
    }

    printk(KERN_EMERG "topics: ioctl\n");
    return ret;
}

static ssize_t topics_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos){
    int fd;
    int err = 0;
    struct file *filp;
    printk(KERN_EMERG "topics: write. i = %d\n", ++i);

    // fd = anon_inode_getfd("liuwei", &internal_fops, NULL,O_CREAT | O_RDWR | O_CLOEXEC);

    fd = get_unused_fd_flags(0);
    // fd = anon_inode_getfd("liuwei", &internal_fops, NULL,O_CREAT | O_RDWR | O_CLOEXEC);

    if ( !filp ) {
        filp = filp_open("liuwei", O_CREAT | O_RDWR | O_CLOEXEC, 0666);
        printk(KERN_EMERG "topics write: filp not exists, create one:%p\n", filp);
    } else {
        printk(KERN_EMERG "topics write: filp has exists, attach one:%p\n", filp);
    }

    if ( IS_ERR(filp) ) {
        err = PTR_ERR(filp);
        printk(KERN_EMERG "topics write: filp fail\n");
        return -1;
    }

    fd_install(fd, filp);

    printk(KERN_EMERG "topics write: filp: fd = %d\n", fd);
    return fd;
}

static struct file_operations topics_flops = {
    .owner          = THIS_MODULE,
    .open           = topics_open,
    .release        = topics_release,
    .unlocked_ioctl = topics_ioctl
};

static int __init topics_init(void){
    int ret;

    topics_cache = kmem_cache_create("topics_cache",
            sizeof(struct topics), 0, 0, NULL);

    if ( !topics_cache ) {
        pr_err("failed to create slab cache\n");
        return -ENOMEM;
    }

    ret = register_chrdev(TOPICS_MAJOR,DEVICE_NAME, &topics_flops);
    if (ret < 0) {
        printk(KERN_EMERG DEVICE_NAME ": can't register major number.\n");
        return ret;
    }
    printk(KERN_EMERG"topics: --- init ---\n");
    return 0;
}

static void __exit topics_exit(void){
    unregister_chrdev(TOPICS_MAJOR, DEVICE_NAME);
    printk(KERN_EMERG"topics: --- exit ---\n");
}


module_init(topics_init);
module_exit(topics_exit);
MODULE_LICENSE("GPL");
