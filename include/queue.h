/********************************************************************
 * queue.h - Queue functions header file
 *
 * $Id$
 ********************************************************************/

#ifndef _QUEUE_H
#define _QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _nodeType
{
	void *data;
	struct _nodeType *next;
} nodeType, *NodeType;


int qAllocNode(NodeType *node, void *data, int size);
int qAddNode(NodeType *head, void *data, int size);
NodeType qGetNextNode(NodeType nextNode);
int qDeleteNode(NodeType *head, NodeType targetNode);

#ifdef __cplusplus
}
#endif

#endif	/* _QUEUE_H */
