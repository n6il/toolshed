/********************************************************************
 * Queue functions
 *
 * These functions implement queue functionality via a linked-list
 * implementation.  A pointer to the data to be associated with a
 * node is passed along with the data size.  A copy of that data is
 * made and referenced by the node.
 *
 * qAllocNode()
 * 		allocates a node and the area for the data given a pointer to
 * 		the data and the data's size in bytes
 *
 * qAddNode()
 * 		 adds a node to a queue
 *
 * qGetNextNode()
 * 		returns the next node in a queue given a node pointer
 *
 * qDeleteNode()
 * 		removes a node from a queue
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>

#include <queue.h>


int qAllocNode(NodeType *node, void *data, int size)
{
    *node = (NodeType)malloc(sizeof(nodeType));

    if (*node == NULL)
    {
		/* error */
        return(1);
    }
    else
    {
        (*node)->data = (void *)malloc(size);
        if ((*node)->data == NULL)
        {
            free(*node);
            return(1);
        }

        /* copy data */
        memcpy((*node)->data, data, size);

        /* no error */
        return(0);
    }
}


int qAddNode(NodeType *head, void *data, int size)
{
    NodeType node;
    NodeType p = *head;

    /* allocate node */
    if (qAllocNode(&node, data, size) != 0)
    {
        return(1);
    }

    /* populate node data structure */
	node->next = NULL;

    /* if head is NULL, then this will be the 1st element in the queue */
    if (*head == NULL)
    {
        *head = node;
    }
    else
    /* walk the queue and insert this node at the end */
    {
        while (p->next != NULL)
        {
            p = p->next;
        }
        p->next = node;
    }

    return(0);
}


NodeType qGetNextNode(NodeType nextNode)
{
    if (nextNode == NULL)
    {
        return(NULL);
    }

    nextNode = nextNode->next;

    return(nextNode);
}	


int qDeleteNode(NodeType *head, NodeType targetNode)
{
    NodeType p = *head;
    NodeType prev = *head;

    /* take 1st case where head may be targetNode */
    if (*head == targetNode)
    {
        *head = (*head)->next;
        free(targetNode->data);
        free(targetNode);
        return(0);
    }

    /* other case: walk the queue to find the node to delete */
    while (p->next != NULL)
    {
        if (p == targetNode)
        {
            /* targetNode has been found */
            prev->next = p->next;
            free(p->data);
            free(p);
            return(0);
        }

        prev = p;
        p = p->next;
    }

    return(1);
}

