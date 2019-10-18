#ifndef __KMOD_H_
#define __KMOD_H_

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module for elevator");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 100 
#define PERMS 0644
#define PARENT NULL
#define KMFLAGS (__GFP_RECLAIM | __GFP_IO | __GFP_FS)

enum STATE { IDLE, OFFLINE, LOADING, UP, DOWN };
enum P_TYPE { ADULT, CHILD, ROOM, BELL };

typedef struct passenger_type
{
    P_TYPE type;
    int s_floor;
    int d_floor;

} passenger;

typedef struct elevator_type 
{    
    struct list_head p[10];         // passengers
    STATE status;
    int w_units;
    int p_units;
    int halves;
    int c_floor;

} elevator;

typedef struct floors_type
{
    struct list_head waiting;
    int level;

} floors;

typedef struct building_type
{
    floors f[10];
    elevator e;
    int current;

} building;

static int elevator_init(void);
static void elevator_exit(void);

int elevator_open(struct inode *sp_inode, struct file *sp_file);
ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset);
int release(struct inode *sp_inode, struct file *sp_file);

int calcWeight(elevator *e);
int calcPass(elevator *e);
int hasSpace(elevator *e);





#endif
