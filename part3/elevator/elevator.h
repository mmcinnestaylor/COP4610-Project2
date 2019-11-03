#ifndef __ELEVATOR_H_
#define __ELEVATOR_H_

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/list.h>

#define KMFLAGS (__GFP_RECLAIM | __GFP_IO | __GFP_FS)
#define MAX_FLOOR 10
#define MAX_W 30
#define MAX_P 10
#define M_SLEEP 2
#define L_SLEEP 1 //not used, how do you simulate a constraint using ssleep()?

typedef enum { IDLE, OFFLINE, LOADING, UP, DOWN } STATE;
typedef enum { ADULT, CHILD, ROOM, BELL } P_TYPE;

typedef struct thread_params
{
    struct task_struct *kthread;
    struct mutex mutx;

} tp;

typedef struct passenger_type
{ 
    struct list_head node;
    P_TYPE type;
    int s_floor;
    int d_floor;

} passenger;

typedef struct elevator_type 
{    
    struct list_head p[10];         // passengers for each floor; added using d_floor as index
    STATE status;
    int w_units;
    int p_units;
    int adult;         //number of adults
    int child;          //number of child
    int room;           //number of room service
    int bell;           //number of bell hops
    int _current;
    int next;       
    int shutdown;   //1 if shutdown has been called
    int direction; // 1 if going up, 0 if going down

} elevator;

typedef struct floors_type
{
    struct list_head waiting;
    int level;
    int w_units;
    int p_units;
    int adult;
    int child;
    int room;
    int bell;
    int serviced;

} floors;

typedef struct building_type
{
    floors f[10];
    elevator *e;

} building;

//  Kernel Module Function Prototypes
static int elevator_init(void);
static void elevator_exit(void);

int elevator_open(struct inode *sp_inode, struct file *sp_file);
ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset);
int elevator_release(struct inode *sp_inode, struct file *sp_file);

void initEThread(tp *param);
int runElevator(void *data);

// Elevator Function Prototypes
void initElevator(elevator *e);
char* getState(elevator *e);
int hasSpace(elevator *e);
int pDirection(passenger *p);
passenger* find(building *b);       //for optimization (if we get there)

void initBuilding(building *b, elevator *e);
void addPass(building *b, passenger *p);
void delPass(building *b, passenger *p);
void movePass(building *b, passenger *p);
int getPass(passenger *p);
int getWeight(passenger *p);
int canFit(passenger *p, elevator *e);
int checkFloor(building *b, int i);
int checkBuilding(building *b);
int checkElevator(elevator *e);
int printStats(building *b, char *buf);

// Syscall Function Prototypes
void link_syscalls(void);
void unlink_syscalls(void);




void initElevator(elevator *e)
{
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        INIT_LIST_HEAD(&e->p[i]);
    }
    
    e->status = OFFLINE;
    e->_current = 0;
    e->next = 1;
    e->w_units = 0;
    e->p_units = 0;
    e->adult = 0;
    e->child = 0;
    e->room = 0;
    e->bell = 0;
    e->shutdown = 0;
    e->direction = 1;
}

void initBuilding(building *b, elevator *e)
{
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        INIT_LIST_HEAD(&b->f[i].waiting);
        b->f[i].level = i;
        b->f[i].w_units = 0;
        b->f[i].p_units = 0;
        b->f[i].adult = 0;
        b->f[i].child = 0;
        b->f[i].room  = 0; 
        b->f[i].bell = 0;
        b->f[i].serviced = 0;
    }
    
    b->e = e;
    b->_current = 0;
}

// always will be added to waiting
void addPass(building *b, passenger *p) 
{
    int w_unit, p_unit;
    w_unit = getWeight(p);
    p_unit = getPass(p);
    
    if (w_unit == -1 || p_unit == -1)
    {
        printk("Invalid p->type value: (%d, %d, %d)\n", p->type, p->s_floor, p->d_floor);
        return;
    }
    
    //printk("addr: %08x, node addr: %08x\n", &p, &p->node);                      
    list_add_tail(&p->node, &b->f[p->s_floor].waiting);
    //printk("addr: %08x, node addr: %08x\n", &p, &p->node);

    b->f[p->s_floor].w_units += w_unit;
    b->f[p->s_floor].p_units += p_unit;

    switch (p->type)
    {
        case ADULT:
            b->f[p->s_floor].adult += 1;
            break;
        case CHILD:
            b->f[p->s_floor].child += 1;
            break;
        case ROOM:
            b->f[p->s_floor].room += 1;
            break;
        case BELL:
            b->f[p->s_floor].bell += 1;
            break;
    }

    //printk("Adding pass type: %d, start: %d, dest: %d\n", p->type + 1, p->s_floor + 1, p->d_floor + 1);
}

// always will be deleted from e->p[i]
void delPass(building *b, passenger *p) 
{
    int w_unit, p_unit;
    w_unit = getWeight(p);
    p_unit = getPass(p);

    if (w_unit == -1 || p_unit == -1)
    {
        printk("Invalid p->type value: (%d, %d, %d)\n", p->type, p->s_floor, p->d_floor);
        return;
    }

    elevator *e = b->e;

    switch (p->type)
    {
        case ADULT:
            e->adult -= 1;
            break;
        case CHILD:
            e->child -= 1;
            break;
        case ROOM:
            e->room -= 1;
            break;
        case BELL:
            e->bell -= 1;
            break;
    }

    e->w_units -= w_unit;
    e->p_units -= p_unit;
    b->f[p->s_floor].serviced++;
    list_del(&p->node);
    kfree(p);
    //printk("Hate when go, love when leave\n");
    //printk("Unloading pass type: %d, start: %d, dest: %d\n", p->type + 1, p->s_floor + 1, p->d_floor + 1);

}


// always will be moved from waiting to e->p[i]
void movePass(building *b, passenger *p)
{
    int w_unit, p_unit;
    w_unit = getWeight(p);
    p_unit = getPass(p);
    
    if (w_unit == -1 || p_unit == -1)
    {
        printk("Invalid p->type value: (%d, %d, %d)\n", p->type, p->s_floor, p->d_floor);
        return;
    }

    elevator *e = b->e;

    switch (p->type)
    {
        case ADULT:
            b->f[p->s_floor].adult -= 1;
            e->adult += 1;
            break;
        case CHILD:
            b->f[p->s_floor].child -= 1;
            e->child += 1;
            break;
        case ROOM:
            b->f[p->s_floor].room -= 1;
            e->room += 1;
            break;
        case BELL:
            b->f[p->s_floor].bell -= 1;
            e->bell += 1;
            break;
    }
    
    b->f[p->s_floor].w_units -= w_unit;
    b->f[p->s_floor].p_units -= p_unit;
    e->w_units += w_unit;
    e->p_units += p_unit;
    list_move_tail(&p->node, &e->p[p->d_floor]);
    //printk("Anotha one\n");
    //printk("Loading pass type: %d, start: %d, dest: %d\n", p->type + 1, p->s_floor + 1, p->d_floor + 1);
}

int getPass(passenger *p)
{
    if (p->type == CHILD || p->type == ADULT)       return 1;
    else if (p->type == ROOM || p->type == BELL)    return 2; // bell or room
    else                                            return -1;
}


int getWeight(passenger *p)
{
    if (p->type == CHILD)       return 1;
    else if (p->type == ADULT)  return 2;
    else if (p->type == ROOM)   return 4;
    else if (p->type == BELL)   return 8;     //bell
    else                        return -1;
}

//get char string of state
char* getState(elevator *e)
{
    if (e->status == IDLE)          return ("Idle");
    else if (e->status == OFFLINE)  return ("Offline");
    else if (e->status == LOADING)  return ("Loading");
    else if (e->status == UP)       return ("Up");
    else if (e->status == DOWN)     return ("Down");
    else                            return ("Not Set");
}

//print building/elevator stats
int printStats(building *b, char *buf)
{
    int len = 0;
    int half = 0;
    elevator *e = b->e;

    if (e->w_units % 2 != 0)
        half = 1;

    len += sprintf(buf + len, "State: %s\n", getState(e));
    len += sprintf(buf + len, "Current Floor: %d\n", e->_current + 1);
    len += sprintf(buf + len, "Next Floor: %d\n", e->next + 1);
    
    if (half)
        len += sprintf(buf + len, "Elevator Load (P/W): %d/%d%s\n", e->p_units, e->w_units/2, ".5");
    else
        len += sprintf(buf + len, "Elevator Load (P/W):  %d/%d\n", e->p_units, e->w_units/2);
    
    int i;
    half = 0;
    for (i = 9; i >= 0; i--)
    {
        if (b->f[i].w_units % 2 != 0)
            half = 1;
        
        if (half)
        {
            len += sprintf(buf + len, 
                "Floor %d: %d/%d%s waiting (P/W) and %d serviced\n", 
                i+1, 
                b->f[i].p_units,
                b->f[i].w_units/2,
                ".5",
                b->f[i].serviced);
            half = 0;
        }
        else
        {
            len += sprintf(buf + len, 
                "Floor %d: %d/%d waiting (P/W) and %d serviced\n", 
                i+1, 
                b->f[i].p_units,
                b->f[i].w_units/2,
                b->f[i].serviced);
        }
    }

    return len;
}

//check if elevator has space
int hasSpace(elevator *e)
{
    if(e->w_units < MAX_W && e->p_units < MAX_P)
        return 1;
    else
        return 0;   
}

// check if the specific passenger can fit in the elevator
int canFit(passenger *p, elevator *e)
{
    if (getWeight(p) + e->w_units <= MAX_W && getPass(p) + e->p_units <= MAX_P)
        return 1;
    else
        return 0;
}

//check if building floor waiting list is empty
int checkFloor(building *b, int i)
{
    if (!list_empty(&b->f[i].waiting))
        return 1;
    else 
        return 0;
}

//check if there is anyone waiting to be serviced throughout the building
int checkBuilding(building *b)
{
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        if (!list_empty(&b->f[i].waiting))
            return 1;
    }

    return 0;
}

//check if there is anyone on the elevator
int checkElevator(elevator *e)
{
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        if (!list_empty(&e->p[i]))
            return 1;
    }

    return 0;
}

/* will find the passenger at the head of the FIFO queue 
to potentially load onto the elevator*/
passenger* find(building *b)
{
    struct list_head *temp;
    passenger *p = NULL;
    elevator *e = b->e;
    
    if (!list_empty(&b->f[e->_current].waiting))
    {
        p = list_entry(&b->f[e->_current].waiting, passenger, node);
        if(canFit(p, e) && pDirection(p) == e->direction) 
            return p;
        else
            return NULL;
    }
    else
        return NULL;
}

//returns true if pass is going up, false otherwise
int pDirection(passenger *p)
{
    if(p->d_floor > p->s_floor)
        return 1;
    else
        return 0;
}
#endif
