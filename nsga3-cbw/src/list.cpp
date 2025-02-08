/* A custom doubly linked list implemenation */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/**
 * @brief Inserts a new element into a doubly linked list.
 *
 * This function creates a new node with the given value and inserts it
 * after the specified node in the doubly linked list. The new node becomes
 * a child of the specified node, and maintains proper parent-child relationships.
 *
 * @param node Pointer to the node after which insertion should occur.
 * @param x The value to be inserted into the new node.
 * @throws Exits with error code 1 if node is NULL.
 */
void insert(list *node, int x)
{
    list *temp;
    if (node == NULL)
    {
        printf("\n Error!! asked to enter after a NULL pointer, hence exiting \n");
        exit(1);
    }
    temp = (list *)malloc(sizeof(list));
    temp->index = x;
    temp->child = node->child;
    temp->parent = node;
    if (node->child != NULL)
    {
        node->child->parent = temp;
    }
    node->child = temp;
    return;
}

/**
 * @brief Deletes a node from the doubly linked list.
 *
 * This function removes the specified node from the list while maintaining
 * the proper parent-child relationships between the remaining nodes.
 * The memory allocated for the deleted node is freed.
 *
 * @param node Pointer to the node to be deleted.
 * @return Pointer to the parent of the deleted node.
 * @throws Exits with error code 1 if node is NULL.
 */
list *del(list *node)
{
    list *temp;
    if (node == NULL)
    {
        printf("\n Error!! asked to delete a NULL pointer, hence exiting \n");
        exit(1);
    }
    temp = node->parent;
    temp->child = node->child;
    if (temp->child != NULL)
    {
        temp->child->parent = temp;
    }
    free(node);
    return (temp);
}