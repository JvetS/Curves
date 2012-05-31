//
//  List.c
//  Curves
//
//  Created by Steven Rossum on 03-05-12.
//  Copyright (c) 2012 Edwin Westerhoud and Steven van Rossum. All rights reserved.
//

#include <stdlib.h>
#include "List.h"

// Allocate a new block of memory to hold a node and return the address of that block.
struct list * listNew(float x, float y)
{
    struct list * l = (struct list *)malloc(sizeof(struct list));
    l->x = x;
    l->y = y;
    l->prev = 0;
    l->next = 0;
    return l;
}

// Free this node and all subnodes.
void listFree(struct list * l)
{
    while (l)
    {
        struct list * tmp = l->next;
        free(l);
        l = tmp;
    }
}

// Add list at end of list l with values x and y.
void listAdd(struct list * l, float x, float y)
{
	struct list * n = 0;
    while (l->next)
        l = l->next;
    
	n = listNew(x, y);
    n->prev = l;
    l->next = n;
}

// Count all subnodes of this list.
int listCount(struct list * l)
{
    int i = 0;
    while (l)
    {
        l = l->next;
        i++;
    }
    return i;
}

// Remove list from sequence and chain prev and next.
struct list * listRemove(struct list * l)
{
    struct list * n = l->next;
    struct list * p = l->prev;
    
    free(l);
    
    if (n)
        n->prev = p;
    if (p)
        p->next = n;
    
    return n;
}

// Remove list at depth d from list l.
struct list * listRemoveAt(struct list * l, int d)
{
    if (!d)
        return listRemove(l);
    
    listRemove(listGet(l, d));
    return l;
}

// Return list at depth d from list l.
struct list * listGet(struct list * l, int d)
{
    while (l && d)
    {
        l = l->next;
        d--;
    }
    return l;
}
