#include "memlib.h"
#include "slist.h"

slist_node* create_node(void* d) {
	slist_node* node;

	node = mem_sbrk(sizeof(node));
	node->next = NULL;
	node->data = d;	

	return node;
}

slist_node* insert_node(slist_node* list, void* data){
	slist_node* node;

    node=create_node(data);
    node->next = list;
	
	return newnode;
}

void remove_node(slist_node* list, slist_node* node){
	
	if(list == NULL || slist_node == NULL)
		return;

	node* cur = list;

	while(cur->next != NULL && cur->next != node)
		cur = cur->next;

	if(cur->next != NULL){
		cur->next = node->next;
	}
}