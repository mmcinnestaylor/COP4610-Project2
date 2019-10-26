#include <linux/kernel.h>
#include "kmod.h"

void initElevator(elevator *e)
{
    int max = 10;
    int i;
    for (i = 0; i < max; i++)
    {
        INIT_LIST_HEAD(e->p[i]);
    }
    
    e->status = IDLE;
    e->current = 0;
    e->next = 0;
    e->w_units = 0;
    e->p_units = 0;
    e->adult = 0;
    e->child = 0;
    e->room = 0;
    e->bell = 0;
}

void initBuilding(elevator *e, building *b)
{
    int max = 10;
    int i;
    for (i = 0; i < max; i++)
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
    
    b->e = e;
    b->current = 0;
}

// always will be added to waiting
void addPass(passenger *p)
{
    int w_units, p_units;
    w_units = getWeight(p);
    p_units = getPass(p);
    
    list_add_tail(&p->node, &b.f[p->s_floor].waiting);

    b.f[p->s_floor].w_units += w_units;
    b.f[p->s_floor].p_units += p_units;

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

}

// always will be moved from waiting to e->p[i]
void movePass(passenger *p)
{

    e.w_units += w_units;
    e.p_units += p_units;
    b.f[p->s_floor].w_units -= w_units;
    b.f[p->s_floor].p_units -= p_units;

    switch (p->type)
    {
        case ADULT:
            e.adult += 1
            break;
        case CHILD:
            break;
        case ROOM:
            break;
        case BELL:
            break;
    }



}

/* will find the passenger at the head of the FIFO queue 
to potentially load onto the elevator*/
list_head* find(building *b)
{
    if(hasSpace(b->e) && canFit(b->f[(b->e).current])
    {
    }
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
    
    len += sprintf(buf + len, "State: %s\n", getState(&e));
    len += sprintf(buf + len, "Current Floor: %d\n", e.current);
    len += sprintf(buf + len, "Next Floor: %d\n", e.next);
    len += sprintf(buf + len, "Elevator Load (P/W):  %d/%d\n", e.p_units, e.w_units/2);
    
    int i;
    for (i = 0; i < MAX_FLOOR; i++)
    {
        len += sprintf(buf + len, 
                "\nFloor %d: %d/%d waiting (P/W) and %d serviced\n", 
                i+1, 
                b.f[i].p_units,
                b.f[i].w_units/2,
                b.f[i].serviced);
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

// 1 if there is room for a passenger and weight limit is below max
// 0 otherwise
int hasSpace(elevator *e)
{
    if(e->w_units < MAX_W && e->p_units < MAX_P)    
        return 1;
    else
        return 0;   
}