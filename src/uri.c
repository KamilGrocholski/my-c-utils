#include "uri.h"
#include "logger.h"
#include <stdlib.h>

void uri_parse(StringView uri, UriComponents *components) {
  if (sv_is_empty(uri)) {
    return;
  }

  ssize_t colon_idx = sv_find(uri, sv_new("://", 3));
  if (colon_idx != -1) {
    components->scheme = sv_sub(uri, 0, colon_idx);
    uri = sv_sub(uri, colon_idx + 3, uri.len);
  } else {
    components->scheme = uri;
    return;
  }

  ssize_t slash_idx = sv_find(uri, sv_new("/", 1));
  if (slash_idx != -1) {
    components->host = sv_sub(uri, 0, slash_idx);
    uri = sv_sub(uri, slash_idx + 1, uri.len);
  } else {
    components->host = uri;
    return;
  }

  ssize_t question_idx = sv_find(uri, sv_new("?", 1));
  if (question_idx != -1) {
    components->path = sv_sub(uri, 0, question_idx);
    uri = sv_sub(uri, question_idx + 1, uri.len);
  } else {
    components->path = uri;
    return;
  }

  ssize_t hash_idx = sv_find(uri, sv_new("#", 1));
  if (hash_idx != -1) {
    components->query = sv_sub(uri, 0, hash_idx);
    uri = sv_sub(uri, hash_idx + 1, uri.len);
  } else {
    components->query = uri;
    return;
  }

  components->fragment = uri;
}

StringBuffer *uri_components_join(UriComponents *components) {
  StringBuffer *uri = sb_new();

  if (!sv_is_empty(components->scheme)) {
    sb_append(uri, components->scheme);
    sb_append(uri, sv_new("://", 3));
  }

  if (!sv_is_empty(components->host)) {
    sb_append(uri, components->host);
    sb_append_char(uri, '/');
  }

  if (!sv_is_empty(components->path)) {
    sb_append(uri, components->path);
    sb_append_char(uri, '?');
  }

  if (!sv_is_empty(components->query)) {
    sb_append(uri, components->query);
    sb_append_char(uri, '#');
  }

  if (!sv_is_empty(components->fragment)) {
    sb_append(uri, components->fragment);
  }

  return uri;
}

size_t uri_query_hash(StringView key) {
  size_t h = 0;
  while (key.len) {
    h = 31 * h + *(key.data)++;
    --key.len;
  }
  return h;
}

UriQueryPairs *uri_query_new() {
  UriQueryPairs *qp = malloc(sizeof(UriQueryPairs));
  if (qp == NULL) {
    logger_log(LOG_FATAL, "uri_query_new malloc err");
  }
  *qp = (UriQueryPairs){
      .cap = URI_QUERY_CAP_INIT,
      .size = 0,
      .buckets = calloc(URI_QUERY_CAP_INIT, sizeof(UriQueryBucket *)),
  };
  if (qp->buckets == NULL) {
    logger_log(LOG_FATAL, "uri_query_new bucckets calloc err");
  }
  return qp;
}

void uri_query_parse(UriQueryPairs *qp, StringView query) {
  StringView raw_pair;
  StringView query_splitter = sv_new("&", 1);
  StringView pair_splitter = sv_new("=", 1);

  while (!sv_is_empty(query)) {
    raw_pair = sv_pop_first_split_by(&query, query_splitter);
    StringView key = sv_pop_first_split_by(&raw_pair, pair_splitter);
    StringView value = raw_pair;
    uri_query_set(qp, key, value);
  }
}

void uri_query_resize(UriQueryPairs *qp, size_t new_cap) {
  UriQueryBucket **new_buckets = calloc(new_cap, sizeof(UriQueryBucket *));
  if (new_buckets == NULL) {
    logger_log(LOG_FATAL, "uri_query_resize new_buckets calloc err");
  }

  for (size_t i = 0; i < qp->cap; ++i) {
    UriQueryBucket *bucket = qp->buckets[i];
    while (bucket) {
      UriQueryBucket *next = bucket->next;
      size_t new_idx = uri_query_hash(bucket->pair.key) % new_cap;
      bucket->next = new_buckets[new_idx];
      new_buckets[new_idx] = bucket;
      bucket = next;
    }
  }

  free(qp->buckets);

  qp->buckets = new_buckets;
  qp->cap = new_cap;
}

void uri_query_set(UriQueryPairs *qp, StringView key, StringView value) {
  size_t hash = uri_query_hash(key);
  size_t idx = hash % qp->cap;
  UriQueryBucket *new_bucket = malloc(sizeof(UriQueryBucket));
  if (new_bucket == NULL) {
    logger_log(LOG_FATAL, "uri_query_set new_bucket malloc err");
  }
  *new_bucket = (UriQueryBucket){
      .next = qp->buckets[idx],
      .pair =
          (UriQueryPair){
              .key = key,
              .value = value,
          },
  };

  qp->buckets[idx] = new_bucket;
  qp->size++;

  if (qp->size > qp->cap * URI_QUERY_RESIZE_AT) {
    uri_query_resize(qp, qp->cap * URI_QUERY_CAP_MULT);
  }
}

StringView uri_query_get(UriQueryPairs *qp, StringView key) {
  size_t hash = uri_query_hash(key);
  size_t idx = hash % qp->cap;
  UriQueryBucket *bucket = qp->buckets[idx];

  while (bucket) {
    if (sv_compare(bucket->pair.key, key)) {
      return bucket->pair.value;
    }
    bucket = bucket->next;
  }

  return sv_new(NULL, 0);
}

bool uri_query_has(UriQueryPairs *qp, StringView key) {
  size_t hash = uri_query_hash(key);
  size_t idx = hash % qp->cap;
  UriQueryBucket *bucket = qp->buckets[idx];

  while (bucket) {
    if (sv_compare(bucket->pair.key, key)) {
      return true;
    }
    bucket = bucket->next;
  }

  return false;
}

StringView uri_query_remove(UriQueryPairs *qp, StringView key) {
  size_t hash = uri_query_hash(key);
  size_t idx = hash % qp->cap;
  UriQueryBucket *bucket = qp->buckets[idx];
  UriQueryBucket *prev_bucket = NULL;

  while (bucket) {
    if (sv_compare(bucket->pair.key, key)) {
      StringView out = (StringView){
          .len = bucket->pair.value.len,
          .data = bucket->pair.value.data,
      };

      if (prev_bucket) {
        prev_bucket->next = bucket->next;
      } else {
        qp->buckets[idx] = bucket->next;
      }

      free(bucket);

      qp->size--;

      return out;
    }

    prev_bucket = bucket;
    bucket = bucket->next;
  }

  return sv_new(NULL, 0);
}

void uri_query_foreach(UriQueryPairs *qp,
                       void (*callback)(StringView key, StringView value)) {
  for (size_t i = 0; i < qp->cap; ++i) {
    UriQueryBucket *bucket = qp->buckets[i];
    while (bucket) {
      callback(bucket->pair.key, bucket->pair.value);
      bucket = bucket->next;
    }
  }
}

void uri_query_free(UriQueryPairs *qp) {
  if (qp == NULL) {
    return;
  }
  for (size_t i = 0; i < qp->cap; ++i) {
    UriQueryBucket *bucket = qp->buckets[i];
    while (bucket) {
      UriQueryBucket *next = bucket->next;
      free(bucket);
      bucket = next;
    }
  }
  free(qp->buckets);
  free(qp);
}

void uri_query_clear(UriQueryPairs *qp) {
  for (size_t i = 0; i < qp->cap; ++i) {
    UriQueryBucket *bucket = qp->buckets[i];
    while (bucket) {
      UriQueryBucket *next = bucket->next;
      free(bucket);
      bucket = next;
    }
    qp->buckets[i] = NULL;
  }
  qp->size = 0;
}
