#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>

typedef struct Arena Arena;

enum {
  front = 0,
  rear = -1,
};

typedef struct Node {
  void* content;
  struct Node* next;
  struct Node* prev;
} Node;

typedef struct LinkedList {
  Arena* arena;
  Node* head;
  Node* tail;
  uint64_t size;
  int64_t capacity;
} LinkedList;

// Initialie and return LinkedList type.
LinkedList list_init();

// Insert data at the front of lnList
void list_insert_at_front(LinkedList* lnList, void* content);

// Insert data at the rear of lnList.
void list_insert_at_rear(LinkedList* lnList, void* content);

// Insert data at a any valid index position in the list.
// Nodes can be accessed from front when pos = 0, 1, 2, ...
// Nodes can be accessed from rear when pos = -1, -2, -3, ...
// Returns NULL on error.
void list_insert_at(LinkedList* lnList, int64_t position, void* content);

// Remove front node and return its data field.
// Returns NULL on error
void* list_remove_from_front(LinkedList* lnList);

// Remove rear node and return its data field.
// Returns NULL on error.
void* list_remove_from_rear(LinkedList* lnList);

// Remove any Node at desired pos and returns its data.
// Nodes can be accessed from front when pos = 0, 1, 2, ... indeces and
// from the rear when pos = -1, -2, -3, ...
// Returns NULL on error.
void* list_remove_from(LinkedList *lnList, int64_t position);

// Modify the content of node at given position.
// Nodes can be accessed from front when pos = 0, 1, 2,... indeces and
// from the rear when pos = -1, -2, -3,...
void list_modify_node_at_pos(LinkedList *lnList, int64_t pos, void* content);

// Returns the content at required index "pos" without modifying the list.
// Returns NULL on failures.
void* list_peekat(LinkedList* lnList, int64_t pos);

// frees the arena allocated for linked list.
void list_free(LinkedList *lnList);
#endif
