/********************************************************************
 * queue.h - Queue functions header file
 *
 * $Id$
 ********************************************************************/
typedef struct _nodeType
{
	void *data;
	struct _nodeType *next;
} nodeType, *NodeType;


int qAllocNode(NodeType *node, void *data, int size);
int qAddNode(NodeType *head, void *data, int size);
NodeType qGetNextNode(NodeType nextNode);
int qDeleteNode(NodeType *head, NodeType targetNode);
