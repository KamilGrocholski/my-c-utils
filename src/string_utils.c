#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "string_utils.h"

// TODO
// better errors
// failed mem alloc should exit(1)
// fix +1 errors, double mallocs

const char *cstr(const char *literal) {
  const char *out = malloc(sizeof(char) * strlen(literal) + 1);
  if (out == NULL) {
    logger_log(LOG_FATAL, "cstr malloc err");
  }
  return out;
}

StringView sv_new(const char *data, size_t len) {
  return (StringView){
      .data = data,
      .len = len,
  };
}

StringView sv_new_from_cstr(const char *data) {
  return (StringView){
      .data = data,
      .len = strlen(data),
  };
}

StringView sv_pop_first_split_by(StringView *src, StringView split_by) {
  StringView out;

  for (size_t i = 0; i < src->len - split_by.len; ++i) {
    if (memcmp(src->data + i, split_by.data, split_by.len) == 0) {
      out = (StringView){
          .len = i,
          .data = src->data,
      };
      src->data = src->data + i + split_by.len;
      src->len = src->len - i - split_by.len;
      return out;
    }
  }

  out = (StringView){
      .len = src->len,
      .data = src->data,
  };
  src->data = "";
  src->len = 0;

  return out;
}

void sv_print(StringView *sv) {
  for (size_t i = 0; i < sv->len; ++i) {
    printf("%c", sv->data[i]);
  }
}

// replace with kmp
ssize_t sv_find(StringView haystack, StringView needle) {
  for (size_t i = 0; i <= haystack.len - needle.len; ++i) {
    if (memcmp(haystack.data + i, needle.data, needle.len) == 0) {
      return (ssize_t)i;
    }
  }
  return -1;
}

bool sv_is_empty(StringView sv) { return sv.len == 0; }

bool sv_starts_with(StringView sv, StringView starts_with) {
  if (sv.len < starts_with.len)
    return false;
  return memcmp(sv.data, starts_with.data, starts_with.len) == 0;
}

bool sv_ends_with(StringView sv, StringView ends_with) {
  if (sv.len < ends_with.len)
    return false;
  return memcmp(sv.data + sv.len - ends_with.len, ends_with.data,
                ends_with.len) == 0;
}

bool sv_contains(StringView haystack, StringView needle) {
  return sv_find(haystack, needle) > -1;
}

bool sv_compare(StringView a, StringView b) {
  if (a.len != b.len)
    return false;
  return memcmp(a.data, b.data, a.len) == 0;
}

bool sv_compare_sb(StringView sv, StringBuffer *sb) {
  if (sv.len != sb->len)
    return false;
  return memcmp(sv.data, sb->data, sv.len) == 0;
}

// Change to idx, count
StringView sv_sub(StringView sv, size_t start, size_t end) {
  return sv_new(sv.data + start, end < sv.len - start ? end : sv.len - start);
}

StringView sv_trim_left(StringView sv) {
  if (sv.len == 0) {
    return sv;
  }
  size_t i = 0;
  while (i < sv.len && isspace(sv.data[i])) {
    i++;
  }
  return (StringView){
      .len = sv.len - i,
      .data = sv.data + i,
  };
}

StringView sv_trim_right(StringView sv) {
  if (sv.len == 0) {
    return sv;
  }
  size_t i = sv.len;
  while (i != 0 && isspace(sv.data[i - 1])) {
    i--;
  }
  return (StringView){
      .len = i,
      .data = sv.data,
  };
}

StringView sv_trim(StringView sv) { return sv_trim_left(sv_trim_right(sv)); }

char *sv_dup(StringView sv) {
  char *str = malloc(sizeof(char) * sv.len + 1);
  if (str == NULL) {
    logger_log(LOG_FATAL, "sv_dup malloc err");
  }
  memcpy(str, sv.data, sv.len);
  str[sv.len] = '\0';
  return str;
}

void sb_resize(StringBuffer *sb, size_t new_cap) {
  sb->data = realloc((char *)sb->data, sizeof(char) * new_cap);
  if (sb->data == NULL) {
    logger_log(LOG_FATAL, "sb_resize realloc err");
  }
  sb->cap = new_cap;
}

StringBuffer *sb_new() {
  StringBuffer *sb = (StringBuffer *)malloc(sizeof(StringBuffer));
  if (sb == NULL) {
    logger_log(LOG_FATAL, "sb_new malloc err");
  }
  *sb = (StringBuffer){
      .data = (char *)malloc(sizeof(char) * SB_INITIAL_CAP),
      .cap = SB_INITIAL_CAP,
      .len = 0,
  };
  if (sb->data == NULL) {
    logger_log(LOG_FATAL, "sb_new->data malloc err");
  }
  return sb;
}

StringBuffer *sb_new_with_custom_cap(size_t cap) {
  StringBuffer *sb = (StringBuffer *)malloc(sizeof(StringBuffer));
  if (sb == NULL) {
    logger_log(LOG_FATAL, "sb_new_with_custom_cap malloc err");
  }
  *sb = (StringBuffer){
      .data = (char *)malloc(sizeof(char) * cap),
      .cap = cap,
      .len = 0,
  };
  if (sb->data == NULL) {
    logger_log(LOG_FATAL, "sb_new_with_custom_cap->data malloc err");
  }
  return sb;
}

StringBuffer *sb_new_from_cstr(const char *cstr) {
  size_t len = strlen(cstr);
  StringBuffer *sb = sb_new_with_custom_cap(len + 1);
  memcpy((char *)sb->data, cstr, len);
  sb->len = len;
  ((char *)sb->data)[len] = '\0';
  return sb;
}

StringBuffer *sb_new_from_sv(StringView view) {
  StringBuffer *sb = malloc(sizeof(StringBuffer));
  if (sb == NULL) {
    logger_log(LOG_FATAL, "sb_new_from_sv malloc err");
  }
  *sb = (StringBuffer){
      .data = sv_dup(view),
      .len = view.len,
      .cap = view.len,
  };

  return sb;
}

// Change to idx, count
StringBuffer *sb_sub(StringBuffer *sb, size_t start, size_t end) {
  if (start >= sb->len) {
    start = sb->len - 1;
  }
  if (end >= sb->len) {
    end = sb->len - 1;
  }
  if (start > end) {
    start = end;
  }
  size_t len = end - start + 1;
  StringBuffer *sub_sb = sb_new_with_custom_cap(len + 1);
  memcpy((char *)sub_sb->data, sb->data + start, len);
  sub_sb->len = len;
  ((char *)sub_sb->data)[len] = '\0';
  return sub_sb;
}

void sb_print(StringBuffer *sb) {
  for (size_t i = 0; i < sb->len; ++i) {
    printf("%c", sb->data[i]);
  }
}

void sb_free(StringBuffer *sb) {
  if (sb != NULL) {
    free((char *)sb->data);
    free(sb);
    sb = NULL;
  }
}

void sb_clear(StringBuffer *sb) {
  memset((char *)sb->data, '\0', sb->len);
  sb->len = 0;
}

bool sb_compare(StringBuffer *a, StringBuffer *b) {
  if (a->len != b->len)
    return false;
  return memcmp(a->data, b->data, a->len) == 0;
}

bool sb_compare_sv(StringBuffer *sb, StringView sv) {
  if (sb->len != sv.len)
    return false;
  return memcmp(sb->data, sv.data, sb->len) == 0;
}

bool sb_is_valid_cstr(StringBuffer *sb) { return sb->data[sb->len] == '\0'; }

bool sb_is_empty(StringBuffer *sb) { return sb->len == 0; }

void sb_append_sb(StringBuffer *sb, StringBuffer *append) {
  if (sb->len + append->len >= sb->cap) {
    sb_resize(sb, sb->cap * 2 + append->len + 1);
  }
  memcpy((char *)sb->data + sb->len, append->data, append->len);
  sb->len += append->len;
  ((char *)sb->data)[sb->len] = '\0';
}

void sb_append(StringBuffer *sb, StringView sv) {
  if (sb->len + sv.len >= sb->cap) {
    sb_resize(sb, sb->cap * 2 + sv.len + 1);
  }
  memcpy((char *)sb->data + sb->len, sv.data, sv.len);
  sb->len += sv.len;
  ((char *)sb->data)[sb->len] = '\0';
}

void sb_append_char(StringBuffer *sb, char ch) {
  if (sb->len + 1 >= sb->cap) {
    sb_resize(sb, sb->cap * 2 + 1);
  }
  ((char *)sb->data)[sb->len] = ch;
  sb->len += 1;
  ((char *)sb->data)[sb->len] = '\0';
}

void sb_insert(StringBuffer *sb, size_t idx, StringView sv) {
  if (idx > sb->len) {
    return;
  }
  if (sb->len + sv.len >= sb->cap) {
    sb_resize(sb, sb->cap * 2 + sv.len + 1);
  }
  memmove((char *)sb->data + sb->len - sv.len, sb->data + idx, sb->len - idx);
  memcpy((char *)sb->data + idx, sv.data, sv.len);
  sb->len += sv.len;
  ((char *)sb->data)[sb->len] = '\0';
}

void sb_remove(StringBuffer *sb, size_t idx, size_t count) {
  if (idx >= sb->len) {
    return;
  }
  count = idx + count >= sb->len ? sb->len - idx : count;
  memmove((char *)sb->data + idx, sb->data + idx + count, sb->len - count);
  sb->len -= count;
  ((char *)sb->data)[sb->len] = '\0';
}

bool sb_file_read(const char *filename, StringBuffer *buffer) {
  FILE *fh = fopen(filename, "rb");
  if (fh == NULL) {
    logger_log(LOG_ERROR, "could not open file '%s'", filename);
    return false;
  }

  fseek(fh, 0L, SEEK_END);
  long len = ftell(fh);
  rewind(fh);

  if (len <= 0) {
    fclose(fh);
    return true;
  }

  sb_resize(buffer, len + 1);

  size_t read_bytes = fread((char *)buffer->data, 1, len, fh);
  if (read_bytes != (size_t)len) {
    fclose(fh);
    logger_log(LOG_ERROR, "error reading file '%s'", filename);
    return false;
  }

  buffer->len = len;
  ((char *)buffer->data)[len] = '\0';

  fclose(fh);
  return true;
}
