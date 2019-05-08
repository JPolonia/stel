#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct node {
	double time;
	int type;
	struct node *next;
};

typedef struct node list;

list *list_new(double time, int type);
list *list_add(list *lst, double time, int type);
list *list_pop(list *lst);
void list_print(list *lst);

#endif
