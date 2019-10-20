#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

#include "kmod.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module for elevator");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 100 
#define PERMS 0644
#define PARENT NULL
#define KMFLAGS (__GFP_WAIT | __GFP_IO | __GFP_FS)

static struct file_operations fops;

static mutex b_mutex;
static mutex e_mutex;

static building b;
static elevator e;

static char* message;
static int read_p;

int elevator_open(struct inode *sp_inode, struct file *sp_file)
{


}

ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{


}

int elevator_release(struct inode *sp_inode, struct file *sp_file)
{
    
}

static int elevator_init(void)
{
    printk(KERN_NOTICE "/proc/%s created\n", ENTRY_NAME);
    fops.open = elevator_open;
    fops.read = elevator_read;
    fops.release = elevator_release;
    
    initElevator(&e);
    initBuilding(&e, &b);
    
    mutex_init(&e_mutex);
    mutex_init(&b_mutex);

    if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
    {
        printk(KERN_WARNING "proc create failed\n");
        remove_proc_entry(ENTRY_NAME, NULL);
        return -ENOMEM;
    }

    return 0;
}
module_init(elevator_init);


static void elevator_exit(void)
{
    remove_proc_entry(ENTRY_NAME, NULL);
    printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);
}
module_exit(elevator_exit);

