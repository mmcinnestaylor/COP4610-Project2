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

}


int calcPass(elevator *e)
{


}

int hasSpace(elevator *e)
{

}
