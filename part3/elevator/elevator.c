#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module for elevator");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1024 
#define PERMS 0644
#define PARENT NULL

static struct file_operations fops;

static tp e_tp;
static elevator e;
static building b;

static char* message;
static int read_p;

void initEThread(tp *param)
{
    mutex_init(&param->mutx);
    param->kthread = kthread_run(runElevator, param, "Elevator thread runnin'\n");
}

int runElevator(void *data)
{
    tp *param = data;
    passenger *p;
    int waiting, servicing;
    struct list_head *ptr, *tmp;
    
    while (!kthread_should_stop())
    {
        if (e.status != OFFLINE) 
        {
            if (mutex_lock_interruptible(&param->mutx) == 0) 
            {      
                if (!list_empty(&b.f[e._current].waiting) || !list_empty(&e.p[e._current]))
                {
                    e.status = LOADING;

                    //empty queue of passengers getting off at floor
                    if (!list_empty(&e.p[e._current]))
                    {
                        list_for_each_safe(ptr, tmp, &e.p[e._current])
                        {
                            p = list_first_entry(ptr, passenger, node);
                            delPass(&b, p);
                        }
                    }
                    if (checkFloor(&b, e._current) && e.shutdown != 1)
                    {
                        list_for_each_safe(ptr, tmp, &b.f[e._current].waiting) 
                        {
                            p = list_first_entry(ptr, passenger, node);
                            
                            if(p != NULL && canFit(p, &e) && pDirection(p) == e.direction) 
                            {
                                printk("(1) Loading pass type: %d, start: %d, end: %d\n",
                                        p->type, p->s_floor, p->d_floor);
                                movePass(&b, p);
                            }
                            else
                                break;
                        }
                    } 
                }


                waiting = checkBuilding(&b);
                servicing = checkElevator(&e);
                
                if (!waiting && !servicing)
                {
                    e.status = IDLE;
                    if (e.shutdown == 1)
                    {
                        if (kthread_stop(e_tp.kthread) != -EINTR)
                            printk("Thread successfully stopped\n");
                    }
                }
                else
                {
                    if(e.next == 0){
                        e._current = e.next; 
                        e.next++;
                        e.direction = 1;
                        e.status = UP;
                    }
                    else if(e.next == 9){            
                        e._current = e.next;
                        e.next--;
                        e.direction = 0;
                        e.status = DOWN;
                    }
                    else if(e.direction == 1) {
                        e._current = e.next;
                        e.next++;
                    }
                    else {
                        e._current = e.next;
                        e.next--;
                    }
                }
            }
            mutex_unlock(&param->mutx);
           

           // if (mutex_lock_interruptible(&param->mutx) == 0) { 
        
                //move to next floor
          //  mutex_unlock(&param->mutx); 
        }
        
        ssleep(M_SLEEP);
    } 
    return 0;
}
/*
int runBuilding(void *data)
{
    tp *param = data;
    passenger *p;
    struct list_head *ptr;
    while (!kthread_should_stop())
    {
        ssleep(L_SLEEP);
        if (mutex_lock_interruptible(&param->mutx) == 0)
        {
            if (checkFloor(e.current))
            {
                list_for_each(ptr, &b.f[e.current].waiting)
                {
                    p = list_entry(ptr, passenger, node);
                    if (canFit(p) && sameDirection(p->d_floor))
                    {
                        movePass(p);
                    }
                }
            }
        }
        mutex_unlock(&param->mutx);
    }

    return 0;
}
*/
extern long (*STUB_issue_request)(int, int, int);
long issue_request(int p_type, int s_floor, int d_floor)
{
    if (p_type < 1 || p_type > 4)       return 1;
    if (s_floor < 1 || s_floor > 10)    return 1;
    if (d_floor < 1 || d_floor > 10)    return 1;
    if (e.shutdown)                     return 1; 

    passenger *p;
    p = kmalloc(sizeof(passenger), KMFLAGS);
    p->type = p_type - 1;
    p->s_floor = s_floor - 1;
    p->d_floor = d_floor - 1;
    INIT_LIST_HEAD(&p->node);
    
    
    if (mutex_lock_interruptible(&e_tp.mutx) == 0)
        addPass(&b, p);
    mutex_unlock(&e_tp.mutx);
    return 0;
}

extern long (*STUB_start_elevator)(void);
long start_elevator(void)
{
    if (e.status != OFFLINE)
        return 1;
    
    if (mutex_lock_interruptible(&e_tp.mutx) == 0)
        e.status = IDLE;
    mutex_unlock(&e_tp.mutx);
    return 0;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void)
{   
    if (e.shutdown) 
        return 1;
    
    if (mutex_lock_interruptible(&e_tp.mutx) == 0)
        e.shutdown = 1;
    mutex_unlock(&e_tp.mutx);
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
    
    int printed = 0;
    while (!printed)
    {
        if (mutex_lock_interruptible(&e_tp.mutx) == 0)
            printed = printStats(&b, message);
        mutex_unlock(&e_tp.mutx);
    }
    
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
    
    int len;
    len = strlen(message); 

    printk(KERN_INFO "proc called read\n");
    copy_to_user(buf, message, len);

    return len;
}

int elevator_release(struct inode *sp_inode, struct file *sp_file)
{
    printk(KERN_NOTICE "proc called release\n");
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
    initBuilding(&b, &e); 
    
    if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
    {
        printk(KERN_WARNING "proc create failed\n");
        remove_proc_entry(ENTRY_NAME, NULL);
        return -ENOMEM;
    }

    initEThread(&e_tp);
    if (IS_ERR(e_tp.kthread))
    {
        printk(KERN_WARNING "error spawning elevator thread");
        remove_proc_entry(ENTRY_NAME, NULL);
        return PTR_ERR(e_tp.kthread);
    }
    
    return 0;
}
module_init(elevator_init);


static void elevator_exit(void)
{
    unlink_syscalls();
    
    struct list_head *ptr, *tmp;
    passenger *p = NULL;
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        if (!list_empty(&e.p[i]))
        {
            list_for_each_safe(ptr, tmp, &e.p[i])
            {
                p = list_entry(ptr, passenger, node);
                list_del(ptr);
                kfree(p);
            } 
        }

        if (!list_empty(&b.f[i].waiting))
        { 
            list_for_each_safe(ptr, tmp, &b.f[i].waiting)
            {
                p = list_entry(ptr, passenger, node);
                list_del(ptr);
                kfree(p);
            }
        }
    }

    mutex_destroy(&e_tp.mutx);

    remove_proc_entry(ENTRY_NAME, NULL);
    printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);
}
module_exit(elevator_exit);
