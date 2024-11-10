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
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../include/arena.h"
#include "../include/utils.h"

typedef enum ListErr {
  LIST_SUCCESS = 0,
  LIST_ALLOC_ERR = -1,
  LIST_FULL_ERR = 1,
  LIST_EMPTY_ERR = 2,
  LIST_INDEX_OUT_OF_BOUNDS_ERR = 3,
} ListErr;

typedef struct Node {
  void* content;
  struct Node* next;
  struct Node* prev;
} Node;

typedef struct List {
  uint64_t len;
  const uint64_t capacity;
  const uint64_t unit_size;
  Arena* arena;
  Node* head;
  Node* tail;
} List;

// Initializes and returns a reference to Node. 
static Node* node_init(Arena **arena, void* content, uint64_t size) {
  Node* node = (Node *)arena_alloc(*arena, sizeof(Node));
  if (content == NULL) {
    DEBUG_PRINT("err: node_init(): can't insert NULL value into list.\n");
    return NULL;
  }
  node->content = arena_alloc(*arena, size);
  if (node->content == NULL) {
    return NULL;
  }
  memcpy(node->content, content, size);
  node->next = NULL;
  node->prev = NULL;
  return node;
}

// Returns a reference to the Node at given index position of linked list.
// Returns NULL if pos is an invalid position in the linked list.
static Node* get_node_using_index(const List *restrict list, int64_t pos) {
  if (pos >= list->len || pos < 0) {
    return NULL;
  }
  size_t index;
  Node *current = NULL;
  if (pos < (list->len) / 2) { // traverse from head 
    index = 0;
    current = list->head;
    while (index < pos) {
      current = current->next;
      ++index;
    }
  } else { // traverse from tail
    index = list->len - 1;
    current = list->tail;
    while (index > pos) {
      current = current->prev;
      --index;
    }
  }
  return current;
}

// Insert newNode after targetNode.
static void insert_after_target_node(List *list, Node *targetNode, Node *newNode) {
  if(targetNode->next != NULL) {
    targetNode->next->prev = newNode;
    newNode->next = targetNode->next;
  } else {
    list->tail = newNode;
  }
  targetNode->next = newNode;
  newNode->prev = targetNode;
  list->len++;
}

// Insert newNode before targetNode.
static void insert_before_target_node(List *list, Node *targetNode, Node *newNode) {
  if (targetNode->prev != NULL) {
    targetNode->prev->next = newNode;
    newNode->prev = targetNode->prev;
  } else {
    list->head = newNode;
  }
  targetNode->prev = newNode;
  newNode->next = targetNode;
  list->len++;
}

// Remove targetNode from list and return its data part
static void* remove_node(List* list, Node* targetNode) {
  if (list->len == 0) {
    DEBUG_PRINT("err: remove_node(): Can't remove from an empty list.\n");
    return NULL;
  }
  void* item = targetNode->content;
  if (list->head == list->tail) {
    list->head = NULL;
    list->tail = NULL;
  } else if(targetNode->next == NULL) {
    list->tail = targetNode->prev;
    list->tail->next->prev = NULL;
    list->tail->next = NULL;
  } else if(targetNode->prev == NULL) {
    list->head = targetNode->next;
    list->head->prev->next = NULL;
    list->head->prev = NULL;
  } else {
    targetNode->prev->next = targetNode->next;
    targetNode->next->prev = targetNode->prev;
    targetNode->next = NULL;
    targetNode->prev = NULL;
  }
  list->len--;
  return item;
}

// Initialize and return LinkedList type.
List list_init(uint64_t capacity, uint64_t unit_size) {
  return (List) {
    .arena = arena_init((sizeof(Node) + unit_size) * ((capacity == 0) ? 2: capacity)),
    .head = NULL,
    .tail = NULL,
    .len = 0,
    .capacity = capacity,
    .unit_size = unit_size
  };
}

// Returns 1 if list is empty, 0 otherwise.
int8_t list_is_empty(const List *restrict list) {
  return (list->len == 0);
}

// Returns 1 if list is full, 0 otherwise.
int8_t list_is_full(const List *restrict list) {
  return (list->len >= list->capacity && list->capacity != 0);
}

// Returns length/size of the List.
uint64_t list_length(const List *restrict list) {
  return list->len;
}

// Insert data at the front of list
// returns 1 on failure
ListErr list_insert_at_front(List* list, void* content) {
  if (list->capacity == 0 || list->len < list->capacity) {
    Node* newNode = node_init(&list->arena, content, list->unit_size);
    if (newNode == NULL) {
      return LIST_ALLOC_ERR;
    }
    if (list->len == 0) {
      list->head = newNode;
      list->tail = list->head;
      list->len++;
    } else {
      list->head->prev = newNode;
      newNode->next = list->head;
      list->head = newNode;
      list->len++;
    }
  } else {
    DEBUG_PRINT("err: list_insert_at_front(): Insertion is not possible, list is full.\n");
    return LIST_FULL_ERR;
  }
  return LIST_SUCCESS;
}

// Insert data at the rear of list.
// returns 1 on failure
ListErr list_insert_at_rear(List* list, void* content) {
  if (list->capacity == 0 || list->len < list->capacity) {
    Node* newNode = node_init(&list->arena, content, list->unit_size);
    if (newNode == NULL) {
      return LIST_ALLOC_ERR;
    }
    if (list->len == 0) {
      list->head = newNode;
      list->tail = list->head;
      list->len++;
    } else {
      newNode->prev = list->tail;
      list->tail->next = newNode;
      list->tail = list->tail->next;
      list->len++;
    }
  } else {
    DEBUG_PRINT("err: list_insert_at_front(): Insertion is not possible, list is full.\n");
    return LIST_FULL_ERR;
  }
  return LIST_SUCCESS;
}

// Insert data at a any valid index position in the list.
// Nodes can be accessed from front when pos = 0, 1, 2, ...
// Nodes can be accessed from rear when pos = -1, -2, -3, ...
// returns 1 on failure
ListErr list_insert_at(List* list, int64_t position, void* content) {
  if (list->capacity == 0 || list->len < list->capacity) {
    Node *newNode = node_init(&list->arena, content, list->unit_size);
    if (newNode == NULL) {
      return LIST_ALLOC_ERR;
    }

    Node *targetNode = NULL;
    if(position == 0) {
      list_insert_at_front(list, content);
    } else if (position == -1 || position == list->len - 1) {
      list_insert_at_rear(list, content);
    } else {
      if (position < 0) {
        position = list->len + position;
        if(position < 0) {
          DEBUG_PRINT("err: ll_insert_at(): pos: %lu is out of bound 0.\n", position);
          return LIST_INDEX_OUT_OF_BOUNDS_ERR;
        }
        if(position == -1) {
          list_insert_at_front(list, content);
        } else {
          targetNode = get_node_using_index(list, position);
          if (targetNode == NULL) {
            DEBUG_PRINT("err: ll_insert_a(): pos: %lu is out of bound 1.\n", position);
            return LIST_INDEX_OUT_OF_BOUNDS_ERR;
          }
          insert_after_target_node(list, targetNode, newNode);
        } 
      } else {
        if(position > list->len) {
          DEBUG_PRINT("err: ll_insert_at_pos(): pos: %lu is out of bound 2.\n", position);
          return LIST_INDEX_OUT_OF_BOUNDS_ERR;
        }
        targetNode = get_node_using_index(list, position);
        if (targetNode == NULL) {
          DEBUG_PRINT("err: ll_insert_at_pos(): pos: %lu is out of bound 3.\n", position);
          return LIST_INDEX_OUT_OF_BOUNDS_ERR;
        }
        insert_before_target_node(list, targetNode, newNode);
      }
    }
  } else {
    DEBUG_PRINT("err: list_insert_at_front(): Insertion is not possible, list is full.\n");
    return LIST_FULL_ERR;
  }
  return LIST_SUCCESS;
}

// Remove front node and return its data field.
void* list_remove_from_front(List* list) {
  if(list->len == 0) {
    DEBUG_PRINT("err: ll_remove_from_front(): Can't remove element from an empty list.\n");
    return NULL;
  }
  return remove_node(list, list->head);
}

// Remove rear node and return its data field.
void* list_remove_from_rear(List* list) {
  if(list->len == 0) {
    DEBUG_PRINT("err: ll_remove_from_rear(): Can't remove element from an empty list.\n");
    return NULL;
  }
  return remove_node(list, list->tail);
}

// Remove any Node at desired pos and returns its data.
// Nodes can be accessed from front when pos = 0, 1, 2, ... indeces and
// from the rear when pos = -1, -2, -3, ...
void* list_remove_from(List *list, int64_t position) {
  if(list->len == 0) {
    DEBUG_PRINT("err: ll_remove_from_pos(): Can't remove element from an empty list.\n");
    return NULL;
  }
  if (position < 0) {
    position = list->len + position;
  }
  if(position > list->len || position < 0) {
    DEBUG_PRINT("err: ll_remove_from_pos(): pos: %lu is an invalid index.\n", position);
    return NULL;
  }
  Node* targetNode = get_node_using_index(list, position);
  if (targetNode == NULL) {
    DEBUG_PRINT("err: ll_remove_at_pos(): pos is out of bound.\n");
    return NULL;
  }
  return remove_node(list, targetNode);
}

// Modify the content of node at given position.
// Nodes can be accessed from front when pos = 0, 1, 2,... indeces and
// from the rear when pos = -1, -2, -3,...
ListErr list_modify_at(List *list, int64_t pos, void* content) {
  if(list->len == 0) {
    DEBUG_PRINT("err: ll_modify_node_at_pos(): Can't modify element from an empty list.\n");
    return LIST_EMPTY_ERR;
  }
  if (pos < 0) {
    pos = list->len + pos;
  }
  if(pos > list->len || pos < 0) {
    DEBUG_PRINT("err: ll_modify_node_at_pos(): pos: %lu is an invalid index.\n", pos);
    return LIST_INDEX_OUT_OF_BOUNDS_ERR;
  }
  Node* targetNode = get_node_using_index(list, pos);
  if (targetNode == NULL) {
    DEBUG_PRINT("err: ll_modify_node_at_pos(): pos is out of bound.\n");
    return LIST_INDEX_OUT_OF_BOUNDS_ERR;
  }
  // targetNode->content = content;
  memcpy(targetNode->content, content, list->unit_size);
  if (targetNode->content == NULL) {
    DEBUG_PRINT("err: ll_modify_node_at_pos(): Memory allocation failed.\n");
    return LIST_ALLOC_ERR;
  }
  return LIST_SUCCESS;
}

// Returns the content at required index "pos" without modifying the list.
void* list_peekat(const List* list, int64_t pos) {
  if(list->len == 0) {
    DEBUG_PRINT("err: ll_peek_at(): Can't remove element from an empty list.\n");
    return NULL;
  }
  if (pos < 0) {
    pos = list->len + pos;
  }
  if(pos > list->len || pos < 0) {
    DEBUG_PRINT("err: ll_peek_at(): pos: %lu is an invalid index.\n", pos);
    return NULL;
  }
  Node* targetNode = get_node_using_index(list, pos);
  if (targetNode == NULL) {
    DEBUG_PRINT("err: ll_peek_at(): pos is out of bound.\n");
    return NULL;
  }
  return targetNode->content;
}

// frees the arena allocated for linked list.
void list_free(List *list) {
  arena_free(list->arena);
  list->len = 0;
}
