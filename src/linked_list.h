#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


struct node {
	void* val;
	struct node* next;
	struct node* prev;
};
typedef struct node node_t;

typedef struct linked_list {
	uint32_t size;
	node_t* start;
	node_t* end;
	uint8_t elem_size;

} linked_list_t;

uint32_t ll_size(linked_list_t* l);
void ll_init(linked_list_t* l, uint8_t elem_size);
void ll_append(linked_list_t* l, void* element);
void* ll_get(linked_list_t* l, uint32_t index);
void* ll_remove(linked_list_t* l, uint32_t index);
void* ll_getFirst(linked_list_t* l);
void* ll_getLast(linked_list_t* l);
void* ll_removeFirst(linked_list_t* l);
void* ll_removeLast(linked_list_t* l);
void ll_print(linked_list_t* l);
void ll_free(linked_list_t* l);


#ifdef LL_IMPLEMENTATION

uint32_t ll_size(linked_list_t* l) {
	return l->size;
}

void ll_init(linked_list_t* l, uint8_t elem_size) {
	l->elem_size = elem_size;
	l->size = 0;
}

void ll_append(linked_list_t* l, void* element) {
	node_t* n = (node_t*) malloc(sizeof(node_t));
	n->val = element;
	
	if (l->size == 0) {
		l->start = n;
		l->end = n;
		n->next = n;
		n->prev = n;
	}
	else {
		l->end->next = n;
		l->end->next->prev = l->end;
		l->end = n;
		l->end->next = l->start;
	}
	++l->size;
}

void ll_print(linked_list_t* l) {
	node_t* cur;
	cur = (l->start);
	printf("%d<- ", *(int*) cur->prev->val);
	for (int i = 0; i < l->size; ++i) {
		printf("%d ", *(int*) cur->val);
		cur = cur->next;
	}
	printf(" ->%d\n", *(int*) cur->val);
}

void* ll_get(linked_list_t* l, uint32_t index) {
	node_t* cur;
	if (index < l->size / 2) {
		cur = l->start;				// traverse from start
		for (int i = 0; i < index; ++i) {
			cur = cur->next;
		}
	}
	else {
		cur = l->end;				// traverse from end
		for (int i = 0; i < l->size - index; ++i) {
			cur = cur->prev;
		}
	}
	return cur->val;
}

void* ll_remove(linked_list_t* l, uint32_t index) {
	if (index == 0) return ll_removeFirst(l);
	if (index == l->size - 1) return ll_removeLast(l);

	node_t* cur;
	void* ret; 
	if (index < l->size / 2) {
		cur = l->start;				// traverse from start
		for (int i = 0; i < index; ++i) {
			cur = cur->next;
		}
	}
	else {
		cur = l->end;				// traverse from end
		for (int i = 0; i < l->size - index; ++i) {
			cur = cur->prev;
		}
	}
	cur->prev->next = cur->next;
	cur->next->prev = cur->prev;
	ret = cur->val;
	free(cur);
	--l->size;
	return ret;

}
void* ll_getFirst(linked_list_t* l) {
	return l->start->val;
}
void* ll_getLast(linked_list_t* l) {
	return l->end->val;
}

void* ll_removeFirst(linked_list_t* l) {
	void* ret = l->start->val;
	l->start->next->prev = l->end;
	l->end->next = l->start->next;
	free(l->start);
	l->start = l->end->next;
	--l->size;
	return ret;
}

void* ll_removeLast(linked_list_t* l) {
	void* ret = l->end->val;
	l->start->prev = l->end->prev;
	l->end->prev->next = l->start;	
	free(l->end);
	l->end = l->end->prev;
	--l->size;
	return ret;
}


void ll_free(linked_list_t* l) {
	node_t* n = l->start;
	for (uint32_t i = 0; i < l->size; ++i) {
		free(n->prev);
		n = n->next;
	}
}
#endif

