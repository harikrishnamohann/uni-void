#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../include/arena.h"

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
  uint64_t capacity;
} LinkedList;


// Initializes and returns a reference to Node. 
static Node* node_init(Arena **arena, void* content) {
  Node* node = (Node *)arena_alloc(*arena, sizeof(Node));
  if (content == NULL) {
    printf("err: node_init(): inserting NULL value into the list.\n");
    return NULL;
  }
  node->content = content;
  node->next = NULL;
  node->prev = NULL;
  return node;
}

// Returns a reference to the Node at given index position of linked list.
// Returns NULL if pos is an invalid position in the linked list.
static Node* get_node_using_index(LinkedList *lnList, int64_t pos) {
  if (pos >= lnList->size || pos < 0) {
    return NULL;
  }
  size_t index;
  Node *current;
  if (pos < (lnList->size) / 2) { // traverse from head 
    index = 0;
    current = lnList->head;
    while (index < pos) {
      current = current->next;
      ++index;
    }
  } else { // traverse from tail
    index = lnList->size - 1;
    current = lnList->tail;
    while (index > pos) {
      current = current->prev;
      --index;
    }
  }
  return current;
}

// Insert newNode after targetNode.
static void insert_after_target_node(LinkedList *lnList, Node *targetNode, Node *newNode) {
  if(targetNode->next != NULL) {
    targetNode->next->prev = newNode;
    newNode->next = targetNode->next;
  } else {
    lnList->tail = newNode;
  }
  targetNode->next = newNode;
  newNode->prev = targetNode;
  lnList->size++;
}

// Insert newNode before targetNode.
static void insert_before_target_node(LinkedList *lnList, Node *targetNode, Node *newNode) {
  if (targetNode->prev != NULL) {
    targetNode->prev->next = newNode;
    newNode->prev = targetNode->prev;
  } else {
    lnList->head = newNode;
  }
  targetNode->prev = newNode;
  newNode->next = targetNode;
  lnList->size++;
}

// Remove targetNode from lnList and return its data part
static void* remove_node(LinkedList* lnList, Node* targetNode) {
  if (lnList->size == 0) {
    printf("err: remove_node(): List is empty. Execution is terminated.\n");
    return NULL;
  }
  void* item = targetNode->content;
  // targetNode->content = NULL;
  if (lnList->head == lnList->tail) {
    lnList->head = NULL;
    lnList->tail = NULL;
  } else if(targetNode->next == NULL) {
    lnList->tail = targetNode->prev;
    lnList->tail->next->prev = NULL;
    lnList->tail->next = NULL;
  } else if(targetNode->prev == NULL) {
    lnList->head = targetNode->next;
    lnList->head->prev->next = NULL;
    lnList->head->prev = NULL;
  } else {
    targetNode->prev->next = targetNode->next;
    targetNode->next->prev = targetNode->prev;
    targetNode->next = NULL;
    targetNode->prev = NULL;
  }
  lnList->size--;
  return item;
}

// Initialize and return LinkedList type.
LinkedList list_init(uint64_t capacity) {
  return (LinkedList) {
    .arena = arena_init(sizeof(Node) * 2),
    .head = NULL,
    .tail = NULL,
    .size = 0,
    .capacity = capacity,
  };
}

// Insert data at the front of lnList
void list_insert_at_front(LinkedList* lnList, void* content) {
  Node* newNode = node_init(&lnList->arena, content);
  if (lnList->size == 0) {
    lnList->head = newNode;
    lnList->tail = lnList->head;
    lnList->size++;
  } else {
    lnList->head->prev = newNode;
    newNode->next = lnList->head;
    lnList->head = newNode;
    lnList->size++;
  }
}

// Insert data at the rear of lnList.
void list_insert_at_rear(LinkedList* lnList, void* content) {
  Node* newNode = node_init(&lnList->arena, content);
  if (lnList->size == 0) {
    lnList->head = newNode;
    lnList->tail = lnList->head;
    lnList->size++;
  } else {
    newNode->prev = lnList->tail;
    lnList->tail->next = newNode;
    lnList->tail = lnList->tail->next;
    lnList->size++;
  }
}

// Insert data at a any valid index position in the list.
// Nodes can be accessed from front when pos = 0, 1, 2, ...
// Nodes can be accessed from rear when pos = -1, -2, -3, ...
void list_insert_at(LinkedList* lnList, int64_t position, void* content) {
  Node *newNode = node_init(&lnList->arena, content);
  Node *targetNode;
  if (position < 0) {
    position = lnList->size + position;
    if(position < 0) {
      printf("err: ll_insert_at_pos(): pos: %lu is out of bound 0.\n", position);
      return;
    }
    if(position == -1) {
      list_insert_at_front(lnList, content);
    } else {
      targetNode = get_node_using_index(lnList, position);
      if (targetNode == NULL) {
        printf("err: ll_insert_at_pos(): pos: %lu is out of bound 1.\n", position);
        return;
      }
      insert_after_target_node(lnList, targetNode, newNode);
    }
  } else if(position == 0) {
    list_insert_at_front(lnList, content);
  } else if(position == lnList->size) {
    list_insert_at_rear(lnList, content);
  } else {
    if(position > lnList->size) {
      printf("err: ll_insert_at_pos(): pos: %lu is out of bound 2.\n", position);
      return;
    }
    targetNode = get_node_using_index(lnList, position);
    if (targetNode == NULL) {
      printf("err: ll_insert_at_pos(): pos: %lu is out of bound 3.\n", position);
      return;
    }
    insert_before_target_node(lnList, targetNode, newNode);
  }
}

// Remove front node and return its data field.
void* list_remove_from_front(LinkedList* lnList) {
  if(lnList->size == 0) {
    printf("err: ll_remove_from_front(): Can't remove element from an empty list.\n");
    return NULL;
  }
  return remove_node(lnList, lnList->head);
}

// Remove rear node and return its data field.
void* list_remove_from_rear(LinkedList* lnList) {
  if(lnList->size == 0) {
    printf("err: ll_remove_from_rear(): Can't remove element from an empty list.\n");
    return NULL;
  }
  return remove_node(lnList, lnList->tail);
}

// Remove any Node at desired pos and returns its data.
// Nodes can be accessed from front when pos = 0, 1, 2, ... indeces and
// from the rear when pos = -1, -2, -3, ...
void* list_remove_from(LinkedList *lnList, int64_t position) {
  if(lnList->size == 0) {
    printf("err: ll_remove_from_pos(): Can't remove element from an empty list.\n");
    return NULL;
  }
  if (position < 0) {
    position = lnList->size + position;
  }
  if(position > lnList->size || position < 0) {
    printf("err: ll_remove_from_pos(): pos: %lu is an invalid index.\n", position);
    return NULL;
  }
  Node* targetNode = get_node_using_index(lnList, position);
  if (targetNode == NULL) {
    printf("err: ll_remove_at_pos(): pos is out of bound.\n");
    return NULL;
  }
  return remove_node(lnList, targetNode);
}

// Modify the content of node at given position.
// Nodes can be accessed from front when pos = 0, 1, 2,... indeces and
// from the rear when pos = -1, -2, -3,...
void list_modify_node_at_pos(LinkedList *lnList, int64_t pos, void* content) {
  if(lnList->size == 0) {
    printf("err: ll_modify_node_at_pos(): Can't modify element from an empty list.\n");
    return;
  }
  if (pos < 0) {
    pos = lnList->size + pos;
  }
  if(pos > lnList->size || pos < 0) {
    printf("err: ll_modify_node_at_pos(): pos: %lu is an invalid index.\n", pos);
    return;
  }
  Node* targetNode = get_node_using_index(lnList, pos);
  if (targetNode == NULL) {
    printf("err: ll_modify_node_at_pos(): pos is out of bound.\n");
    return;
  }
  if (targetNode->content != NULL) {
    free(targetNode->content);
  } 
  targetNode->content = content;
  if (targetNode->content == NULL) {
    printf("err: ll_modify_node_at_pos(): Memory allocation failed.\n");
    return;
  }
}

// Returns the content at required index "pos" without modifying the list.
void* list_peekat(LinkedList* lnList, int64_t pos) {
  if(lnList->size == 0) {
    printf("err: ll_peek_at(): Can't remove element from an empty list.\n");
    return NULL;
  }
  if (pos < 0) {
    pos = lnList->size + pos;
  }
  if(pos > lnList->size || pos < 0) {
    printf("err: ll_peek_at(): pos: %lu is an invalid index.\n", pos);
    return NULL;
  }
  Node* targetNode = get_node_using_index(lnList, pos);
  if (targetNode == NULL) {
    printf("err: ll_peek_at(): pos is out of bound.\n");
    return NULL;
  }
  return targetNode->content;
}

// frees the arena allocated for linked list.
void list_free(LinkedList *lnList) {
  Node* tmp = lnList->head;
  // while (tmp != NULL) {
    // free(tmp->content);
    // tmp = tmp->next;
  // }
  arena_free(lnList->arena);
  lnList->size = 0;
}
