#include "uri.h"

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
