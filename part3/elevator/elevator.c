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

static tp e_tp;     // thread paramater struct; includes kthread and mutex
static elevator e;  // elevator
static building b;  // building

static char* message;
static int read_p;

// initialize thread parameter; init mutx, and call kthread_run
void initEThread(tp *param)
{
    mutex_init(&param->mutx);
    param->kthread = kthread_run(runElevator, param, "Elevator thread runnin'\n");
}

// kthread function; main loop
int runElevator(void *data)
{
    tp *param = data;
    passenger *p = NULL;
    int waiting, servicing;
    struct list_head *ptr, *tmp;
    waiting = 0;
    servicing = 0;
    ptr = NULL;
    tmp = NULL;

    while (!kthread_should_stop())
    {
        if (mutex_lock_interruptible(&param->mutx) == 0) 
        {
            if (e.status != OFFLINE) 
            {
                if (!list_empty(&b.f[e._current].waiting) || !list_empty(&e.p[e._current]))
                {
                    e.status = LOADING;

                    //empty queue of passengers getting off at floor
                    if (!list_empty(&e.p[e._current]))
                    {
                        list_for_each_safe(ptr, tmp, &e.p[e._current])
                        {
                            p = list_entry(ptr, passenger, node);
                            if (p->d_floor == e._current)
                                delPass(&b, p);
                            //else {
                                //printk("no match. d: %d, e: %d, (s: %d)\n", p->d_floor, e._current, p->s_floor);
                                //printk("addr: %08x, node addr: %08x\n", p, &p->node);
                            //}

                        
                        }
                        ptr = NULL; tmp = NULL;
                    }
                    if (checkFloor(&b, e._current) && e.shutdown != 1)
                    {
                        list_for_each_safe(ptr, tmp, &b.f[e._current].waiting) 
                        {
                           // printk("ptr add: %08x, ptr value %08x\n", &ptr, ptr);
                            p = list_entry(ptr, passenger, node);
                           // printk("p add: %08x, p value: %08x, p->node addr: %08x\n", &p, p, &p->node);
                            if (canFit(p, &e) && pDirection(p) == e.direction) 
                            {
                                if (p->s_floor == e._current)
                                    movePass(&b, p);
                                //else{
                                 //   printk("no match. s: %d, e: %d, (d: %d)\n", p->s_floor, e._current, p->d_floor);
                                //}
                            }
                            else
                                break;
                        }
                        ptr = NULL; tmp = NULL;
                    } 
                }
            }
        }
        mutex_unlock(&param->mutx);
        ssleep(L_SLEEP);

        if (mutex_lock_interruptible(&param->mutx) == 0)
        {
            if (e.status != OFFLINE)
            {
                waiting = checkBuilding(&b);
                servicing = checkElevator(&e);
                
                if (!servicing && !waiting)
                {
                    e.status = IDLE;
                }
                else if (!servicing && e.shutdown)
                {
                    e.status = OFFLINE;           
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
                        e.status = UP;
                        e._current = e.next;
                        e.next++;
                    }
                    else {
                        e.status = DOWN;
                        e._current = e.next;
                        e.next--;
                    }
                }
            }
        }
        mutex_unlock(&param->mutx);
        ssleep(M_SLEEP);
    } 
    return 0;
}


/*
 *  issue reuqest - syscall
 *  @ int
 *  @ int
 *  @ int
 *  - check valid range on all params or if shutdown is set, return 1 if true
 *  - allocate mem for passenger node, init list head
 *  - call addPass in locked region
 */
extern long (*STUB_issue_request)(int, int, int);
long issue_request(int p_type, int s_floor, int d_floor)
{
    if (p_type < 1 || p_type > 4)       return 1;
    if (s_floor < 1 || s_floor > 10)    return 1;
    if (d_floor < 1 || d_floor > 10)    return 1;
    if (e.shutdown)                     return 1; 

    passenger *p;
    p = kmalloc(sizeof(passenger), __GFP_RECLAIM);
    //printk("Addr of new pass: %08x, addr of new p->node: %08x\n", &p, &p->node);
    p->type = p_type - 1;
    p->s_floor = s_floor - 1;
    p->d_floor = d_floor - 1;
    INIT_LIST_HEAD(&p->node);
    
    
    if (mutex_lock_interruptible(&e_tp.mutx) == 0)
        addPass(&b, p);
    mutex_unlock(&e_tp.mutx);
    return 0;
}

/*
 *  start elevator - syscall
 *  @ void
 *  - get lock
 *  - if elevator is offline, return 1
 *  - else init elevator status, floors, and shutdown status
 *
 */
extern long (*STUB_start_elevator)(void);
long start_elevator(void)
{
    int s = 0;
    if (mutex_lock_interruptible(&e_tp.mutx) == 0)
    {
        if (e.status != OFFLINE)
            s = 1;
        else {
            e.status = IDLE;
            e._current = 0;
            e.next = 1;
            e.shutdown = 0;
        }   
    }
    mutex_unlock(&e_tp.mutx);
    
    return s;
}


/*
 *  stop elevator - syscall
 *  @ void
 *  - get lock
 *  - if shutdown is set, return 1
 *  - else set shutdown
 */
extern long (*STUB_stop_elevator)(void);
long stop_elevator(void)
{   
    int s = 0;
    if (mutex_lock_interruptible(&e_tp.mutx) == 0)
    {
        if (e.shutdown) 
            s = 1;
        else 
            e.shutdown = 1;
    }
    mutex_unlock(&e_tp.mutx);
    
    return s;
}


//link syscall function pointers to locally defined functions
void link_syscalls(void)
{
    STUB_issue_request = issue_request;
    STUB_start_elevator = start_elevator;
    STUB_stop_elevator = stop_elevator;
}


//unlink syscall function pointers back to NULL
void unlink_syscalls(void)
{
    STUB_issue_request = NULL;
    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
}

//on proc open, get lock and print elevator/building stats
int elevator_open(struct inode *sp_inode, struct file *sp_file)
{   
    //printk(KERN_INFO "proc called open\n");
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

    //printk(KERN_INFO "proc called read\n");
    copy_to_user(buf, message, len);

    return len;
}

int elevator_release(struct inode *sp_inode, struct file *sp_file)
{
    //printk(KERN_NOTICE "proc called release\n");
    kfree(message);
    return 0;
}

// link syscalls, link fops, init elevator and building, and init eThread
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

// stop kthread, unlink syscalls, deallocate all passenger nodes, destroy mutx and proc entry
static void elevator_exit(void)
{

    if (kthread_stop(e_tp.kthread) != -EINTR)
        printk("Thread successfully stopped\n");

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
