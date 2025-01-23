/* Rank assignment routine */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/* Function to assign rank size pop_size*/
void assign_rank(population *new_pop)
{
    int flag;
    int i;
    int end;
    int front_size;
    int rank = 1;
    list *orig;
    list *cur;
    list *temp1, *temp2;
    orig = (list *)malloc(sizeof(list));
    cur = (list *)malloc(sizeof(list));
    front_size = 0;
    orig->index = -1;
    orig->parent = NULL;
    orig->child = NULL;
    cur->index = -1;
    cur->parent = NULL;
    cur->child = NULL;
    temp1 = orig;
    for (i = 0; i < popsize; i++)
    {
        insert(temp1, i);
        temp1 = temp1->child;
    }
    do
    {
        if (orig->child->child == NULL)
        {
            new_pop->ind[orig->child->index].rank = rank;
            break;
        }
        temp1 = orig->child;
        insert(cur, temp1->index);
        front_size = 1;
        temp2 = cur->child;
        temp1 = del(temp1);
        temp1 = temp1->child;
        do
        {
            temp2 = cur->child;
            do
            {
                end = 0;
                flag = check_dominance(&(new_pop->ind[temp1->index]), &(new_pop->ind[temp2->index]));
                if (flag == 1)
                {
                    insert(orig, temp2->index);
                    temp2 = del(temp2);
                    front_size--;
                    temp2 = temp2->child;
                }
                if (flag == 0)
                {
                    temp2 = temp2->child;
                }
                if (flag == -1)
                {
                    end = 1;
                }
            } while (end != 1 && temp2 != NULL);
            if (flag == 0 || flag == 1)
            {
                insert(cur, temp1->index);
                front_size++;
                temp1 = del(temp1);
            }
            temp1 = temp1->child;
        } while (temp1 != NULL);
        temp2 = cur->child;
        do
        {
            new_pop->ind[temp2->index].rank = rank;
            temp2 = temp2->child;
        } while (temp2 != NULL);

        temp2 = cur->child;
        do
        {
            temp2 = del(temp2);
            temp2 = temp2->child;
        } while (cur->child != NULL);
        rank += 1;
    } while (orig->child != NULL);
    free(orig);
    free(cur);
    return;
}

/* Function to assign rank size 2*pop_size*/
void assign_rank_mixedpop(population *new_pop)
{
    int flag;
    int i;
    int end;
    int front_size;
    int rank = 1;
    list *orig;
    list *cur;
    list *temp1, *temp2;
    orig = (list *)malloc(sizeof(list));
    cur = (list *)malloc(sizeof(list));
    front_size = 0;
    orig->index = -1;
    orig->parent = NULL;
    orig->child = NULL;
    cur->index = -1;
    cur->parent = NULL;
    cur->child = NULL;
    temp1 = orig;
    for (i = 0; i < 2 * popsize; i++)
    {
        insert(temp1, i);
        temp1 = temp1->child;
    }
    do
    {
        if (orig->child->child == NULL)
        {
            new_pop->ind[orig->child->index].rank = rank;
            break;
        }
        temp1 = orig->child;
        insert(cur, temp1->index);
        front_size = 1;
        temp2 = cur->child;
        temp1 = del(temp1);
        temp1 = temp1->child;
        do
        {
            temp2 = cur->child;
            do
            {
                end = 0;
                flag = check_dominance(&(new_pop->ind[temp1->index]), &(new_pop->ind[temp2->index]));
                if (flag == 1)
                {
                    insert(orig, temp2->index);
                    temp2 = del(temp2);
                    front_size--;
                    temp2 = temp2->child;
                }
                if (flag == 0)
                {
                    temp2 = temp2->child;
                }
                if (flag == -1)
                {
                    end = 1;
                }
            } while (end != 1 && temp2 != NULL);
            if (flag == 0 || flag == 1)
            {
                insert(cur, temp1->index);
                front_size++;
                temp1 = del(temp1);
            }
            temp1 = temp1->child;
        } while (temp1 != NULL);
        temp2 = cur->child;
        do
        {
            new_pop->ind[temp2->index].rank = rank;
            temp2 = temp2->child;
        } while (temp2 != NULL);

        temp2 = cur->child;
        do
        {
            temp2 = del(temp2);
            temp2 = temp2->child;
        } while (cur->child != NULL);
        rank += 1;
    } while (orig->child != NULL);
    free(orig);
    free(cur);
    return;
}
