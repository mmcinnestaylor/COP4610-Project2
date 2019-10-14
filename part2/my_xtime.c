#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/time.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple module featuring proc read");

#define ENTRY_NAME "timed"
#define ENTRY_SIZE 100
#define PERMS 0644
#define PARENT NULL

static struct file_operations fops;
static struct timespec curr_t;
static struct timespec prev_t;
static char *message;
static int read_p;
static int called;

int myxtime_proc_open(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_INFO "proc called open\n");
	
	read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
    
    if (message == NULL) {
		printk(KERN_WARNING "myxtime_proc_open");
		return -ENOMEM;
	}
	
    return 0;
}

ssize_t myxtime_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {

    int len; 
    
    read_p = !read_p;
	if (read_p)
		return 0;
    
    curr_t = current_kernel_time();
    len = sprintf(message, "current time: %lu.%lu\n",  (long)curr_t.tv_sec, curr_t.tv_nsec);
    
    if (called != 0)
    {
        prev_t = timespec_sub(curr_t, prev_t);
        len += sprintf(message + len, "elapsed time: %lu.%lu\n", (long)prev_t.tv_sec, prev_t.tv_nsec);
    }
    	
    printk(KERN_INFO "proc called read\n");
	copy_to_user(buf, message, len);
    
    prev_t = curr_t;
    if (called == 0)
        called = 1;
    
    return len;
}

int myxtime_proc_release(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_NOTICE "proc called release\n");
	kfree(message);
    return 0;
}

static int myxtime_init(void) {
	printk(KERN_NOTICE "/proc/%s created\n",ENTRY_NAME);
	fops.open = myxtime_proc_open;
	fops.read = myxtime_proc_read;
	fops.release = myxtime_proc_release;
    called = 0;

	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk(KERN_WARNING "proc create\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}
	
	return 0;
}
module_init(myxtime_init);

static void myxtime_exit(void) {
	remove_proc_entry(ENTRY_NAME, NULL);
	printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);
}
module_exit(myxtime_exit);
