#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"

list *list_new(double time, int type) {
	list *lst = malloc(sizeof(*lst));
	if (!lst)
		return NULL;
	lst->next = NULL;
	lst->time = time;
	lst->type = type;
	return lst;
}

list *list_add(list *lst, double time, int type) {
	if (!lst)
		return list_new(time, type);
	struct node *n = list_new(time, type);
	if (!n)
		return NULL;
	struct node *t, *prev;
	for (t = prev = lst; t->next != NULL; prev = t, t = t->next) {
		if (t->time > time) {
			n->next = t;
			if (t == prev)
				return n;
			prev->next = n;
			return lst;
		}
	}
	if (t->time < time) {
		t->next = n;
		return lst;
	}
	prev->next = n;
	n->next = t;
	return lst;
}

list *list_pop(list *lst) {
	if (!lst)
		return NULL;
	struct node *n = lst->next;
	free(lst);
	return n;
}

void list_print(list *lst) {
	if (!lst)
		return;
	struct node *n = lst;
	for (int i = 0; n != NULL; n = n->next, i++)
		printf("Element %d:\n\ttime - %f\n\ttype - %d\n\n", i, n->time, n->type);
	return;
}

