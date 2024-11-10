/* 
## Notes ##
This implementation is not stable and it may have critical bugs.
It is not thread safe. Use it with caution!

This is an implementation of doubly linked list using Arena allocator.
It includes basic operations like insertion, removal, peeking,
and modification of elements, as well as custom index-based access
from both ends of the list.

## About ##
This small project is a part of "hobby's_".
Author: Harikrishna Mohan
Contact: harikrishnamohan@proton.me
Started on: 20-10-2024

## Example usage ##
Initializing a list with a capacity of 20 elements
of type:- struct Employee:
  List employees = list_init(20, sizeof(struct Employee));

Initializing an infinite list (capacity = 0 or DYNAMIC) of integers:
  List numbers = list_init(DYNAMIC, sizeof(int));

insertion functions:
  - list_insert_at()
  - list_insert_at_front()
  - list_insert_at_rear() 
Example:
  int data = 42;
  list_insert_at_front(&my_list, &data);

Removal functions:
  - list_remove_from()
  - list_remove_from_front()
  - list_remove_from_rear() 
Example:
  int data = *(int*)list_remove_from(&my_list, 0);

Peeking:
  int peeked = *(int*)list_peekat(&my_list, FRONT);
  printf("Front element: %d\n", peeked);

Modifying the elements:
  int x = 40;
  list_modify_at(&my_list, -2, &x);

Utility functions:
  - list_is_empty()
  - list_is_full()
  - list_length()

Deallocating the list:
  list_free(&my_list);
*/

#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>

#define FRONT 0
#define REAR -1
#define DYNAMIC 0

typedef enum ListErr {
  LIST_SUCCESS = 0,
  LIST_ALLOC_ERR = -1,
  LIST_FULL_ERR = 1,
  LIST_EMPTY_ERR = 2,
  LIST_INDEX_OUT_OF_BOUNDS_ERR = 3,
} ListErr;

typedef struct Arena Arena;
typedef struct Node Node;

typedef struct List {
  uint64_t len;
  const uint64_t capacity;
  const uint64_t unit_size;
  Arena* arena;
  Node* head;
  Node* tail;
} List;

// Initialie and return LinkedList type.
// use capacity = 0 for a dynamically resizable list.
// unit_size is capacity of a single element that has to be
// inserted in the list.
// example usage: if you want a dynamic list of integers,
// initialize list as: 
// List integers = list_init(0, sizeof(int));
// or a list of 20 employees:
// List employees = list_init(20, sizeof(struct Employee));
List list_init(uint64_t capacity, uint64_t unit_size);

// Insert data at the front of list
ListErr list_insert_at_front(List* list, void* content);

// Insert data at the rear of list.
ListErr list_insert_at_rear(List* list, void* content);

// Insert data at a any valid index position in the list.
// Nodes can be accessed from front when pos = 0, 1, 2, ...
// Nodes can be accessed from rear when pos = -1, -2, -3, ...
ListErr list_insert_at(List* list, int64_t position, void* content);

// Remove front node and return its content field.
// Return NULL on error
void* list_remove_from_front(List* list);

// Remove rear node and return its content field.
// Return NULL on error.
void* list_remove_from_rear(List* list);

// Remove any Node at desired pos and returns its content.
// Nodes can be accessed from front when pos = 0, 1, 2, ... indeces and
// from the rear when pos = -1, -2, -3, ...
// Return NULL on error.
void* list_remove_from(List *list, int64_t position);

// Modify the content of node at given position.
// Nodes can be accessed from front when pos = 0, 1, 2,... indeces and
// from the rear when pos = -1, -2, -3,...
ListErr list_modify_at(List* list, int64_t pos, void* content);

// Returns the content at required index "pos" without modifying the list.
// Returns NULL on failure.
void* list_peekat(const List* list, int64_t pos);

// frees the arena allocated for linked list.
void list_free(List *list);

// Returns 1 if list is empty, 0 otherwise.
int8_t list_is_empty(const List* list);

// Returns length/size of the List.
uint64_t list_length(const List* list); 

// Returns 1 if list is full, 0 otherwise.
int8_t list_is_full(const List* list);

#endif
