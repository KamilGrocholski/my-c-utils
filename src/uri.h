#include "string_utils.h"

#ifndef _URI_H
#define _URI_H

#define URI_QUERY_CAP_INIT 16
#define URI_QUERY_RESIZE_AT 0.75
#define URI_QUERY_CAP_MULT 2

typedef struct {
  StringView scheme;
  StringView host;
  StringView path;
  StringView query;
  StringView fragment;
} UriComponents;

typedef struct {
  StringView key;
  StringView value;
} UriQueryPair;

typedef struct UriQueryBucket {
  UriQueryPair pair;
  struct UriQueryBucket *next;
} UriQueryBucket;

typedef struct {
  UriQueryBucket **buckets;
  size_t size;
  size_t cap;
} UriQueryPairs;

void uri_parse(StringView uri, UriComponents *components);

StringBuffer *uri_components_join(UriComponents *components);

void uri_query_parse(UriQueryPairs *qp, StringView query);
UriQueryPairs *uri_query_new();
void uri_query_resize(UriQueryPairs *qp, size_t new_cap);
void uri_query_free(UriQueryPairs *qp);
void uri_query_clear(UriQueryPairs *qp);
void uri_query_set(UriQueryPairs *qp, StringView key, StringView value);
StringView uri_query_get(UriQueryPairs *qp, StringView key);
StringView uri_query_remove(UriQueryPairs *qp, StringView key);
bool uri_query_has(UriQueryPairs *qp, StringView key);
void uri_query_foreach(UriQueryPairs *qp,
                       void (*callback)(StringView key, StringView value));

#endif // _URI_H
