#ifndef __KMOD_H_
#define __KMOD_H_

#include <linux/list.h>

#define KMFLAGS (__GFP_WAIT | __GFP_IO | __GFP_FS)
#define MAX_FLOOR 10
#define MAX_W 30
#define MAX_P 10
#define M_SLEEP 2
#define L_SLEEP 1

enum STATE { IDLE, OFFLINE, LOADING, UP, DOWN };
enum P_TYPE { ADULT, CHILD, ROOM, BELL };

struct list_head
{
    struct list_head *next;
    struct list_head *prev;

};

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
    int current;

} building;

//  Kernel Module Function Prototypes
static int elevator_init(void);
static void elevator_exit(void);

int elevator_open(struct inode *sp_inode, struct file *sp_file);
ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset);
int elevator_release(struct inode *sp_inode, struct file *sp_file);


// Elevator Function Prototypes
void initElevator(elevator *e);
void initBuilding(elevator *e, building *b);
void addPass(passenger *p);
void delPass(passenger *p);
void movePass(passenger *p);
int printStats(char *buf);
int calcWeight(elevator *e);
int calcPass(elevator *e);
int hasSpace(elevator *e);
char* getState(elevator *e);


// Syscall Function Prototypes
void link_syscalls(void);
void unlink_syscalls(void);


#endif
