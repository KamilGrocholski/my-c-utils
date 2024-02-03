#include <stddef.h>

#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

typedef struct {
  void *(*alloc)(size_t bytes, void *context);
  void *(*free)(size_t bytes, void *ptr, void *context);
  void *context;
} Allocator;

#endif // _ALLOCATOR_H
