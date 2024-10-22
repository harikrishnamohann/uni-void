#include <stdio.h>
#include "../include/linked_list.h"

struct Content {
  int id;
  char *name;
};

int main() {
  LinkedList list = linked_list_init();
  struct Content content = {
    .id = 23,
    .name = "hari",
  };
  ll_insert_at_front(&list, &content);
  struct Content* tmp = ll_remove_from_front(&list);
  printf("%s\n", tmp->name);
  ll_remove_from_front(&list);

  // printf("%s\n", ((struct Content*)ll_peekat(&list, 0))->name);
  ll_free(&list);
}
