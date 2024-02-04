#include <stddef.h>
#include <stdlib.h>

#include "allocator.h"

#ifndef _DYNAMIC_ARRAY_H
#define _DYNAMIC_ARRAY_H

#define DA_INITIAL_CAP 16

#define DYNAMIC_ARRAY(T)                                                       \
  struct {                                                                     \
    T *items;                                                                  \
    size_t len;                                                                \
    size_t cap;                                                                \
  }

#define DA_REALLOC(items, new_cap) items = realloc(items, new_cap)

#define da_init(da, T)                                                         \
  da->items = malloc(sizeof(T));                                               \
  da->len = 0;                                                                 \
  da->cap = DA_INITIAL_CAP;

#define da_free(da) free(da->items)

#define da_append(da, item)                                                    \
  if (da->len >= da->cap) {                                                    \
    da->cap = da->cap == 0 ? DA_INITIAL_CAP : da->cap * 2;                     \
    da->items = DA_REALLOC(da->items, da->cap * sizeof(*da->items));           \
  }                                                                            \
  da->items[da->len++] = item;

#endif // _DYNAMIC_ARRAY_H
