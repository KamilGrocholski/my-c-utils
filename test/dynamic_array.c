#include <assert.h>

#include "../src/allocator.h"
#include "../src/dynamic_array.h"

typedef DYNAMIC_ARRAY(int) Values;

void values_swap(Values *values, size_t a, size_t b) {
  int temp = values->items[a];
  values->items[a] = values->items[b];
  values->items[b] = temp;
}

void test_dynamic_array() {
  Values *values;
  da_init(values, int);
  da_append(values, 2);
  assert(values->items[0] == 2 && "should da append");
}
