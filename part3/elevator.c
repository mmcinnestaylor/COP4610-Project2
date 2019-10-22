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
    if(e->w_units < MAX_W)
        return 1;
    else
        return 0;
    
}
