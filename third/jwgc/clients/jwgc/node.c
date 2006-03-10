/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "node.h"

/****************************************************************************/
/* */
/* Internal node construction & destruction functions:           */
/* */
/****************************************************************************/

/*
 * NODE_BATCH_SIZE - the number of nodes to malloc at once to save overhead:
 */

#define  NODE_BATCH_SIZE    100

/*
 * The nodes we have malloced are kept in a linked list of bunches of
 * NODE_BATCH_SIZE nodes.  Nodes points to the first bunch on the list
 * and current_bunch to the last.  All nodes from the first one in the first
 * bunch to the last_node_in_current_bunch_used'th one in the last bunch
 * are in use.  The others have not been used yet.
 */

static struct _bunch_of_nodes {
	struct _bunch_of_nodes *next_bunch;
	Node nodes[NODE_BATCH_SIZE];
} *nodes = NULL;
static struct _bunch_of_nodes *current_bunch = NULL;
static int last_node_in_current_bunch_used = -1;

/*
 *  Internal Routine:
 *
 *    Node *node_create(int opcode)
 *        Effects: Creates a node with opcode opcode and returns a pointer
 *                 to it.  The next pointer of the returned node is NULL.
 *                 If the opcode is STRING_CONSTANT_OPCODE the caller must
 *                 ensure that the string_constant field points to a valid
 *                 string on the heap when node_DestroyAllNodes is called.
 */

static Node *
node_create(opcode)
	int opcode;
{
	Node *result;

	if (!nodes) {
		/*
		 * Handle special case where no nodes allocated yet:
		 */
		current_bunch = nodes = (struct _bunch_of_nodes *)
			malloc(sizeof(struct _bunch_of_nodes));
		nodes->next_bunch = NULL;
		last_node_in_current_bunch_used = -1;
	}

	/*
         * If all nodes allocated so far in use, allocate another
         * bunch of NODE_BATCH_SIZE nodes:
         */
	if (last_node_in_current_bunch_used == NODE_BATCH_SIZE - 1) {
		current_bunch->next_bunch = (struct _bunch_of_nodes *)
			malloc(sizeof(struct _bunch_of_nodes));
		current_bunch = current_bunch->next_bunch;
		current_bunch->next_bunch = NULL;
		last_node_in_current_bunch_used = -1;
	}

	/*
         * Get next not already used node & ready it for use:
         */
	last_node_in_current_bunch_used++;
	result = &(current_bunch->nodes[last_node_in_current_bunch_used]);
	result->opcode = opcode;
	result->next = NULL;

	return (result);
}

/*
 *
 */

void 
node_DestroyAllNodes()
{
	struct _bunch_of_nodes *next_bunch;
	int i, last_node_used_in_this_bunch;

	while (nodes) {
		next_bunch = nodes->next_bunch;
		last_node_used_in_this_bunch = next_bunch ?
			NODE_BATCH_SIZE - 1 : last_node_in_current_bunch_used;
		for (i = 0; i <= last_node_used_in_this_bunch; i++) {
			if (nodes->nodes[i].opcode == STRING_CONSTANT_OPCODE)
				free(nodes->nodes[i].d.string_constant);
			else if (nodes->nodes[i].opcode == VARREF_OPCODE)
				free(nodes->nodes[i].d.string_constant);
			else if (nodes->nodes[i].opcode == VARNAME_OPCODE)
				free(nodes->nodes[i].d.string_constant);
		}
		free(nodes);
		nodes = next_bunch;
	}

	current_bunch = nodes;
}

/****************************************************************************/
/* */
/* Node construction functions:                         */
/* */
/****************************************************************************/

Node *
node_create_string_constant(opcode, text)
	int opcode;
	string text;
{
	Node *n;

	n = node_create(opcode);
	n->d.string_constant = text;
	return (n);
}

Node *
node_create_noary(opcode)
	int opcode;
{
	Node *n;

	n = node_create(opcode);
	return (n);
}

Node *
node_create_unary(opcode, arg)
	int opcode;
	Node *arg;
{
	Node *n;

	n = node_create(opcode);
	n->d.nodes.first = arg;
	return (n);
}

Node *
node_create_binary(opcode, first_arg, second_arg)
	int opcode;
	Node *first_arg;
	Node *second_arg;
{
	Node *n;

	n = node_create(opcode);
	n->d.nodes.first = first_arg;
	n->d.nodes.second = second_arg;
	return (n);
}

/****************************************************************************/
/* */
/* Node utility functions:                           */
/* */
/****************************************************************************/

/*
 *    Node *reverse_list_of_nodes(Node *list)
 *        Modifies: the nodes on the linked list list
 *        Effects: Reverses the linked list list and returns it.
 *                 This is done by modifing the next pointers of the
 *                 list elements to point to the previous node & returning
 *                 the address of the (previously) last node.
 */

Node *
reverse_list_of_nodes(list)
	Node *list;
{
	Node *next_node;
	Node *head = NULL;

	while (list) {
		next_node = list->next;

		/*
		 * Add the node list to the beginning of linked list head:
		 */
		list->next = head;
		head = list;

		list = next_node;
	}

	return (head);
}
