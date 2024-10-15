#include <stdio.h>
#include "../include/arena.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
  char* content;
  struct Node* next;
  struct Node* prev;
} Node;

typedef struct LinkedList {
  Arena* arena;
  Node* head;
  Node* tail;
  size_t size;
} LinkedList;

// Initializes and returns a reference to Node. 
static Node* node_init(Arena **arena, char* content) {
  Node* node = (Node *)arena_alloc(*arena, sizeof(Node));
  if (content == NULL) {
    printf("err: node_init(): inserting NULL value into the list.\n");
    exit(1);
  }
  node->content = content;
  node->next = NULL;
  node->prev = NULL;
  return node;
}

// Returns a reference of Node at first occurance of key in the linked list,
// returns NULL if key is not found.
// It uses linear search algo for now.
static Node* search_node_using_key(Node* head, char* key) {
  while(head != NULL) {
    if (!strcmp(head->content, key)) {
      return head; 
    }
    head = head->next;
  }
  return NULL;
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
static char* remove_node(LinkedList* lnList, Node* targetNode) {
  if (lnList->size == 0) {
    printf("err: remove_node(): List is empty. Execution is terminated.\n");
    exit(1);
  }
  char* item = targetNode->content;
  targetNode->content = 0;
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

// Initialie and return LinkedList type.
LinkedList linked_list_init() {
  return (LinkedList) {
    .arena = arena_init(sizeof(Node) * 2),
    .head = NULL,
    .tail = NULL,
    .size = 0,
  };
}

// Insert data at the front of lnList
void ll_insert_at_front(LinkedList* lnList, char* content) {
  Node* newNode = node_init(&lnList->arena, strdup(content));
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
void ll_insert_at_rear(LinkedList* lnList, char* content) {
  Node* newNode = node_init(&lnList->arena, strdup(content));
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

// Insert data next to first occurance of key in lnList.
void ll_insert_after_key(LinkedList* lnList, char* key, char* content) {
  Node* targetNode = search_node_using_key(lnList->head, key);
  if(targetNode == NULL) {
    printf("err: ll_insert_after_key(): targetNode is NULL.\n");
    exit(1);
  }
  Node* newNode = node_init(&lnList->arena, strdup(content));
  insert_after_target_node(lnList, targetNode, newNode);
}

// Insert data before first occurance of key in lnList.
void ll_insert_before_key(LinkedList* lnList, char* key, char* content) {
  Node* targetNode = search_node_using_key(lnList->head, key);
  if(targetNode == NULL) {
    printf("err: ll_insert_after_key(): targetNode is NULL.\n");
    exit(1);
  }
  Node* newNode = node_init(&lnList->arena, strdup(content));
  insert_before_target_node(lnList, targetNode, newNode);
}

// Insert data at a any valid index position in the list.
// Nodes can be accessed from front when pos = 0, 1, 2, ...
// Nodes can be accessed from rear when pos = -1, -2, -3, ...
void ll_insert_at_pos(LinkedList* lnList, int64_t pos, char* content) {
  Node *newNode = node_init(&lnList->arena, strdup(content));
  Node *targetNode;
  if (pos < 0) {
    pos = lnList->size + pos;
    if(pos < 0) {
      printf("err: ll_insert_at_pos(): pos: %lu is out of bound 0.\n", pos);
      exit(0);
    }
    if(pos == -1) {
      ll_insert_at_front(lnList, content);
    } else {
      targetNode = get_node_using_index(lnList, pos);
      if (targetNode == NULL) {
        printf("err: ll_insert_at_pos(): pos: %lu is out of bound 1.\n", pos);
        exit(0);
      }
      insert_after_target_node(lnList, targetNode, newNode);
    }
  } else if(pos == 0) {
    ll_insert_at_front(lnList, content);
  } else if(pos == lnList->size) {
    ll_insert_at_rear(lnList, content);
  } else {
    if(pos > lnList->size) {
      printf("err: ll_insert_at_pos(): pos: %lu is out of bound 2.\n", pos);
      exit(0);
    }
    targetNode = get_node_using_index(lnList, pos);
    if (targetNode == NULL) {
      printf("err: ll_insert_at_pos(): pos: %lu is out of bound 3.\n", pos);
      exit(0);
    }
    insert_before_target_node(lnList, targetNode, newNode);
  }
}

// Remove front node and return its data field.
char* ll_remove_from_front(LinkedList* lnList) {
  if(lnList->size == 0) {
    printf("err: ll_remove_from_front(): Can't remove element from an empty list.\n");
    exit(1);
  }
  return remove_node(lnList, lnList->head);
}

// Remove rear node and return its data field.
char* ll_remove_from_rear(LinkedList* lnList) {
  if(lnList->size == 0) {
    printf("err: ll_remove_from_rear(): Can't remove element from an empty list.\n");
    exit(1);
  }
  return remove_node(lnList, lnList->tail);
}

// Remove the first occurance of key from linked list and return it.
char* ll_remove_using_key(LinkedList* lnList, char* key) {
  if(lnList->size == 0) {
    printf("err: ll_remove_using_key(): Can't remove element from an empty list.\n");
    exit(1);
  }
  Node* targetNode = search_node_using_key(lnList->head, key);
  if(targetNode == NULL) {
    printf("err: ll_remove_using_key(): key is not present in the list.\n");
    exit(1);
  }
  return remove_node(lnList, targetNode);
}

// Remove any Node at desired pos and returns its data.
// Nodes can be accessed from front when pos = 0, 1, 2, ... indeces and
// from the rear when pos = -1, -2, -3, ...
char* ll_remove_from_pos(LinkedList *lnList, int64_t pos) {
  if(lnList->size == 0) {
    printf("err: ll_remove_from_pos(): Can't remove element from an empty list.\n");
    exit(1);
  }
  if (pos < 0) {
    pos = lnList->size + pos;
  }
  if(pos > lnList->size || pos < 0) {
    printf("err: ll_remove_from_pos(): pos: %lu is an invalid index.\n", pos);
    exit(1);
  }
  Node* targetNode = get_node_using_index(lnList, pos);
  if (targetNode == NULL) {
    printf("err: ll_remove_at_pos(): pos is out of bound.\n");
    exit(0);
  }
  return remove_node(lnList, targetNode);
}

// Prints out the linked list to stdout.
void ll_display(const LinkedList* lnList) {
  Node* head = lnList->head;
  if (lnList->size > 0) {
    printf("size of linked list: %lu\n", lnList->size);
    while (head != NULL) {
      printf("[%s]::[prev:%p]:[current:%p]:[next:%p]\n", head->content, head->prev, head, head->next);
      head = (Node*)head->next;
    }
  } else {
    printf("There are no elements in the given reference: %p.\nError! list is empty.\n", lnList);
  }
}

// Modify the content of node at given position.
// Nodes can be accessed from front when pos = 0, 1, 2,... indeces and
// from the rear when pos = -1, -2, -3,...
void ll_modify_node_at_pos(LinkedList *lnList, int64_t pos, char* content) {
  if(lnList->size == 0) {
    printf("err: ll_modify_node_at_pos(): Can't modify element from an empty list.\n");
    exit(1);
  }
  if (pos < 0) {
    pos = lnList->size + pos;
  }
  if(pos > lnList->size || pos < 0) {
    printf("err: ll_modify_node_at_pos(): pos: %lu is an invalid index.\n", pos);
    exit(1);
  }
  Node* targetNode = get_node_using_index(lnList, pos);
  if (targetNode == NULL) {
    printf("err: ll_modify_node_at_pos(): pos is out of bound.\n");
    exit(0);
  }
  if (targetNode->content != NULL) {
    free(targetNode->content);
  } 
  targetNode->content = strdup(content);
  if (targetNode->content == NULL) {
    printf("err: ll_modify_node_at_pos(): Memory allocation failed.\n");
    exit(1);
  }
}

// Returns the content at required index "pos" without modifying the list.
char* ll_peekat(LinkedList* lnList, int64_t pos) {
  if(lnList->size == 0) {
    printf("err: ll_peek_at(): Can't remove element from an empty list.\n");
    exit(1);
  }
  if (pos < 0) {
    pos = lnList->size + pos;
  }
  if(pos > lnList->size || pos < 0) {
    printf("err: ll_peek_at(): pos: %lu is an invalid index.\n", pos);
    exit(1);
  }
  Node* targetNode = get_node_using_index(lnList, pos);
  if (targetNode == NULL) {
    printf("err: ll_peek_at(): pos is out of bound.\n");
    exit(0);
  }
  return targetNode->content;
}

// frees the arena allocated for linked list.
void ll_free(LinkedList *lnList) {
  Node* tmp = lnList->head;
  while (tmp != NULL) {
    free(tmp->content);
    tmp = tmp->next;
  }
  arena_free(lnList->arena);
  lnList->size = 0;
}

int main() {
  LinkedList list = linked_list_init();
  ll_insert_at_front(&list, "hari");
  ll_insert_at_rear(&list, "neil");
  ll_insert_at_pos(&list, -1, "anandu");
  ll_insert_before_key(&list, "hari", "shervin j");
  for (int i = 0; i < list.size; i++) {
    printf("%s\n", ll_peekat(&list, i));
  }
  ll_free(&list);
  return 0;
}
