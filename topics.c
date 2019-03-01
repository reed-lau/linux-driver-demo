#include <linux/shmem_fs.h>
#include <linux/anon_inodes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/falloc.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include "topics.h"

static int (*shmem_zero_setup_ptr)(struct vm_area_struct *) = NULL;

int shmem_zero_setup(struct vm_area_struct *vma)
{
        if (!shmem_zero_setup_ptr)
                    shmem_zero_setup_ptr = kallsyms_lookup_name("shmem_zero_setup");
            return shmem_zero_setup_ptr(vma);
}

#define TOPICS_MAJOR 231
#define DEVICE_NAME "topics"

static int i = 0;

/**
 * struct topics - The topics related shared memory 
 * @name:    The topic of the shared memory
 * @file     The backing file
 * @size     The size of one topic of the shared memory
 */

struct topics {
    char name[1024];
    struct file *file;
    size_t size;
};

static DEFINE_MUTEX(topics_mutex);

static struct kmem_cache *topics_cache __read_mostly;

/**
 * topics_open() - Open Shared memory of one topic name 
 * @inode:   The backing file's inode
 * @file:    The backing file
 *
 * Return: 0 if successful, or another code if unsuccessful.
 */
static int topics_open(struct inode *inode, struct file *file){
    struct topics *topic;
    int ret;

    ret = generic_file_open(inode, file);
    if (ret)
        return ret;

    topic = kmem_cache_zalloc(topics_cache, GFP_KERNEL);
    if ( !topic )
        return -ENOMEM;

    memcpy(topic->name, "topic", 5);
    file->private_data = topic;

    printk(KERN_EMERG "topics: open\n");
    return 0;
}

/**
 * topics_release() - Release the shared memory of one topic name
 * @inode:   The backing file's inode
 * @file:    The backing file
 *
 * Return: 0 if successful. If it is anything else, go have a coffee
 * and try again
 */
static int topics_release(struct inode *inode, struct file *file) {
    struct topics *topic = file->private_data;

    printk(KERN_EMERG "topics: release\n");
    if ( topic->file )
        fput(topic->file);

    kmem_cache_free(topics_cache, topic);

    return 0;
}


static int topics_mmap(struct file *file, struct vm_area_struct *vma) {
    struct topics *topic = file->private_data; 
    // mutex_lock(&topics_mutex);
    // mutex_unlock(&topics_mutex);

    int ret;

    /*
    if (!topic->size) {
        ret = -EINVAL;
        goto out;
    }
    */

    // must set shared among process
    if (!(vma->vm_flags & VM_SHARED)) {
        ret = -EINVAL;
        goto out;
    }

    printk(KERN_EMERG "topics: mmap %d\n", vma->vm_flags);

    // name and size has not been set by ioctl()
    // init it here
    if (!topic->file) {
        struct file *vmfile;

        printk(KERN_EMERG "topics: 1 -----\n");

        vmfile = shmem_file_setup("xyzw", 1024, vma->vm_flags);
        if (IS_ERR(vmfile)) {
            ret = PTR_ERR(vmfile);
            goto out;
        }

        topic->file = vmfile;
        vmfile->f_mode |= FMODE_LSEEK;
        printk(KERN_EMERG "topics: 2 -------\n");
    }

    // increase refcnt
    get_file(topic->file);

    ret = shmem_zero_setup(vma);
    if (ret) {
        fput(topic->file);
        goto out;
    }

    if (vma->vm_file)
        fput(vma->vm_file);

    vma->vm_file = topic->file;
    printk(KERN_EMERG "topics: mmap ok mmap called [%d] times\n", i++);

out:
    return ret;
}

static long topics_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct topics *topic = file->private_data;
    int ret = -ENOTTY;
    printk(KERN_EMERG "topics: ioctl\n");
    return ret;
}

static struct file_operations topics_flops = {
    .owner          = THIS_MODULE,
    .open           = topics_open,
    .release        = topics_release,
    .mmap           = topics_mmap,
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
