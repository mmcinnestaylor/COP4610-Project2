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
#define L_SLEEP 1

enum STATE { IDLE, OFFLINE, LOADING, UP, DOWN };
enum P_TYPE { ADULT, CHILD, ROOM, BELL };

typedef struct thread_params
{
    struct task_struct *kthread;
    struct mutex mutx;

} tp;

typedef struct passenger_type
{
    P_TYPE type;
    struct list_head node;
    int s_floor;
    int d_floor;

} passenger;

typedef struct elevator_type 
{    
    struct list_head p[10];         // passengers
    STATE status;
    int w_units;
    int p_units;
    int adult;
    int child;
    int room;
    int bell;
    int current;
    int next;
    int shutdown;
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
    int current;

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
int calcWeight(elevator *e);
int calcPass(elevator *e);
int hasSpace(elevator *e);
int pDirection(passenger *p);
passanger* find();

void initBuilding(building *b);
void addPass(passenger *p);
void delPass(passenger *p);
void movePass(passenger *p);
int getPass(passenger *p);
int getWeight(passenger *p);
int canFit(passenger *p);
int checkFloor(int i);

int printStats(char *buf);

// Syscall Function Prototypes
void link_syscalls(void);
void unlink_syscalls(void);




void initElevator(elevator *e)
{
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        INIT_LIST_HEAD(e->p[i]);
    }
    
    e->status = OFFLINE;
    e->current = 0;
    e->next = 0;
    e->w_units = 0;
    e->p_units = 0;
    e->adult = 0;
    e->child = 0;
    e->room = 0;
    e->bell = 0;
    e->shutdown = 0;
}

void initBuilding(building *b)
{
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        INIT_LIST_HEAD(b->f[i].waiting);
        b->f[i].level = i;
        b->f[i].w_units = 0;
        b->f[i].p_units = 0;
        b->f[i].adult = 0;
        b->f[i].child = 0;
        b->f[i].room  = 0; 
        b->f[i].bell = 0;
        b->f[i].serviced = 0;
    }

    b->current = 0;
}

// always will be added to waiting
void addPass(passenger *p)
{
    int w_unit, p_unit;
    w_unit = getWeight(p);
    p_unit = getPass(p);
    
    list_add_tail(&p->node, &b.f[p->s_floor].waiting);

    b.f[p->s_floor].w_units += w_unit;
    b.f[p->s_floor].p_units += p_unit;

    switch (p->type)
    {
        case ADULT:
            b.f[p->s_floor].adult += 1;
            break;
        case CHILD:
            b.f[p->s_floor].child += 1;
            break;
        case ROOM:
            b.f[p->s_floor].room += 1;
            break;
        case BELL:
            b.f[p->s_floor].bell += 1;
            break;
    }
}

// always will be deleted from e->p[i]
void delPass(passenger *p)
{
    int w_unit, p_unit;
    w_unit = getWeight(p);
    p_unit = getPass(p);

    switch (p->type)
    {
        case ADULT:
            e.adult -= 1;
            break;
        case CHILD:
            e.child -= 1;
            break;
        case ROOM:
            e.room -= 1;
            break;
        case BELL:
            e.bell -= 1;
            break;
    }

    e.w_units -= w_unit;
    e.p_units -= p_unit;
    b.f[p->s_floor].serviced++;
    list_del(&p->node);
    kfree(p);
    printk("Hate when go, love when leave");
}


// always will be moved from waiting to e->p[i]
void movePass(passenger *p)
{
    int w_unit, p_unit;
    w_unit = getWeight(p);
    p_unit = getPass(p);   
    
    switch (p->type)
    {
        case ADULT:
            b.f[p->s_floor].adult -= 1
            e.adult += 1;
            break;
        case CHILD:
            b.f[p->s_floor].child -= 1
            e.child += 1;
            break;
        case ROOM:
            b.f[p->s_floor].room -= 1
            e.room += 1;
            break;
        case BELL:
            b.f[p->s_floor].bell -= 1
            e.bell += 1;
            break;
    }
    
    b.f[p->s_floor].w_units -= w_unit;
    b.f[p->s_floor].p_units -= p_unit;
    e.w_units += w_unit;
    e.p_units += p_unit;
    list_move_tail(&p->node, &e.p[p->d_floor]);
    printk("Anotha one");
}

int getPass(passenger *p)
{
    if (p->type == CHILD || p->type == ADULT)   return 1;
    else                                        return 2; // bell or room
}


int getWeight(passenger *p)
{
    if (p->type == CHILD)       return 1;
    else if (p->type == ADULT)  return 2;
    else if (p->type == ROOM)   return 4;
    else                        return 8;     //bell
}


char* getState(elevator *e)
{
    if (e->status == IDLE)          return ("Idle");
    else if (e->status == OFFLINE)  return ("Offline");
    else if (e->status == LOADING)  return ("Loading");
    else if (e->status == UP)       return ("Up");
    else if (e->status == DOWN)     return ("Down");
    else                            return ("Not Set");
}

int printStats(char *buf)
{
    int len = 0;
    int half = 0;
    
    if (e.w_units % 2 != 0)
        half = 1;

    len += sprintf(buf + len, "State: %s\n", getState(&e));
    len += sprintf(buf + len, "Current Floor: %d\n", e.current);
    len += sprintf(buf + len, "Next Floor: %d\n", e.next);
    
    if (half)
        len += sprintf(buf + len, "Elevator Load (P/W): %d/%d%s\n", e.p_units, e.w_units/2, ".5");
    else
        len += sprintf(buf + len, "Elevator Load (P/W):  %d/%d\n", e.p_units, e.w_units/2);
    
    int i;
    half = 0;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        if (b.f[i].w_units % 2 != 0)
            half = 1;
        
        if (half)
        {
            len += sprintf(buf + len, 
                "\nFloor %d: %d/%d%s waiting (P/W) and %d serviced\n", 
                i+1, 
                b.f[i].p_units,
                b.f[i].w_units/2,
                ".5",
                b.f[i].serviced);
            half = 0;
        }
        else
        {
            len += sprintf(buf + len, 
                "\nFloor %d: %d/%d waiting (P/W) and %d serviced\n", 
                i+1, 
                b.f[i].p_units,
                b.f[i].w_units/2,
                b.f[i].serviced);
        }
    }

    return len;
}

int calcWeight(elevator *e)
{
    e->w_units = e->child + e->adult * 2 + e->room * 4 + e->bell * 8;
    return e->w_units;
}


int calcPass(elevator *e)
{
    e->p_units = e->child + e->adult + e->room * 2 + e->bell * 2;
    return e->p_units;
}

int hasSpace(elevator *e)
{
    if(e->w_units < MAX_W && e->p_units < MAX_P)
        return 1;
    else
        return 0;   
}

int canFit(passenger *p)
{
    if (getWeight(p) + e.w_units <= MAX_W && getPass(p) + e.p_units <= MAX_P)
        return 1;
    else
        return 0;
}

int checkFloor(int i)
{
    if (!list_empty(&b.f[i].waiting))
        return 1;
    else 
        return 0;
}

/* will find the passenger at the head of the FIFO queue 
to potentially load onto the elevator*/
passenger* find()
{
    struct list_head temp;
    passenger *p = NULL;

    p = list_entry(temp, passenger, &(b.f[e.current]).waiting);
    if(canFit(p) && pDirection(p) == e.direction) 
        return p;
    else
        return NULL;
}

int pDirection(passenger *p)
{
    if(p->d_floor > p->s_floor)
        return 1;
    else
        return 0;
}
#endif
