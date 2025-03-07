#include "../include/uni-void.h"

void swap(int *x, int *y) { *x = *x ^ *y; *y = *x ^ *y; *x = *x ^ *y; }

void push_key(Key stk[], int16_t *top, Key key) {
  if (*top == STK_SIZE - 1) { // stack full
    for (int i = 1; i < STK_SIZE; i++) {
      stk[i - 1] = stk[i];
    }
    (*top)--;
  }  
  stk[++(*top)] = key;
}

Key pop_key(Key stk[], int16_t *top) {
  if (*top == -1) {
    return key_invalid;
  }
  return stk[(*top)--];
}

void make_radomized_array(int* arr, size_t size) {
  uint32_t pos;
  arr[0] = 0;
  for (size_t i = 1; i < size; i++) {
    arr[i] = i;
    pos = rand() % i;
    swap(&arr[pos], &arr[i]);
  }
}
