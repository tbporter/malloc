#ifndef __SLIST_H
#define __SLIST_H

typedef struct slist_node_struct {
	struct node_struct* next;
	void* data;	
} slist_node;


slist_node* create_node(void* d);
slist_node* insert_node(slist_node* list, void* data);
void remove_node(slist_node* list, slist_node* node);

#endif