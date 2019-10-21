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
#define ENTRY_SIZE 1024 
#define PERMS 0644
#define PARENT NULL

#define KMFLAGS (__GFP_WAIT | __GFP_IO | __GFP_FS)
#define MAX_WEIGHT 30
#define MAX_PASS 10
#define MAX_FLOOR 10
#define M_SLEEP 2
#define L_SLEEP 1

static struct file_operations fops;

static mutex b_mutex;
static mutex e_mutex;

static building b;
static elevator e;

static char* message;
static int read_p;


extern long (*STUB_issue_request)(int, int, int);
long issue_request(int p_type, int s_floor, int d_floor)
{

    return 0;
}

extern long (*STUB_start_elevator)(void);
long start_elevator(void)
{

    return 0;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void)
{
    return 0;
}

void link_syscalls(void)
{
    STUB_issue_request = issue_request;
    STUB_start_elevator = start_elevator;
    STUB_stop_elevator = stop_elevator;
}

void unlink_syscalls(void)
{
    STUB_issue_request = NULL;
    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
}

int elevator_open(struct inode *sp_inode, struct file *sp_file)
{   
    printk(KERN_INFO "proc called open\n");
    read_p = 1;

    message = kmalloc(sizeof(char) * ENTRY_SIZE, KMFLAGS);
    if (message == NULL)
    {
        printk(KERN_WARNING "error in elevator_open");
        return -ENOMEM;
    }

    return 0;
}

ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{
    read_p = !read_p;
    if (read_p)
        return 0;   
  
    printk(KERN_INFO "proc called read\n");
    copy_to_user(buf, message, len);

    return len;
}

int elevator_release(struct inode *sp_inode, struct file *sp_file)
{
    prink(KERN_NOTICE "proc called release\n");
    kfree(message);
    return 0;
}

static int elevator_init(void)
{
    printk(KERN_NOTICE "/proc/%s created\n", ENTRY_NAME);
    link_syscalls();

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
    unlink_syscalls();
    remove_proc_entry(ENTRY_NAME, NULL);
    printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);
}
module_exit(elevator_exit);

