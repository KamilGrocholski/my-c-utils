#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "string_utils.h"

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
  printf("\"");
  for (size_t i = 0; i < sv->len; ++i) {
    printf("%c", sv->data[i]);
  }
  printf("\"");
  printf("\n");
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

bool sv_is_valid_cstr(StringView sv) { return sv.data[sv.len] == '\0'; }

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

StringView sv_sub(StringView sv, size_t idx, size_t count) {
  return sv_new(sv.data + idx, count < sv.len - idx ? count : sv.len - idx);
}

StringView sv_trim_left(StringView sv) {
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
  size_t i = sv.len - 1;
  while (i > 0 && isspace(sv.data[i])) {
    i--;
  }
  return (StringView){
      .len = i + 1,
      .data = sv.data,
  };
}

StringView sv_trim(StringView sv) { return sv_trim_left(sv_trim_right(sv)); }

char *sv_dup(StringView sv) {
  char *str = malloc(sizeof(char) * sv.len + 1);
  memcpy(str, sv.data, sv.len);
  str[sv.len] = '\0';
  return str;
}

bool sv_file_read(const char *filename, StringView *sv) {
  FILE *fh = fopen(filename, "rb");
  if (fh == NULL) {
    logger_log(LOG_WARNING, "could not open file '%s'", filename);
    return 0;
  }

  fseek(fh, 0L, SEEK_END);
  sv->len = ftell(fh);

  char *b = (char *)malloc(sv->len);

  if (b == NULL) {
    logger_log(LOG_ERROR, "mem alloc err not open file '%s'", filename);
    fclose(fh);
    return false;
  }

  if (sv->len == 0) {
    fclose(fh);
    sv->data = b;
    return true;
  }

  rewind(fh);

  if (fread(b, sv->len, 1, fh) != 1) {
    free(b);
    fclose(fh);
    return false;
  }

  sv->data = b;

  fclose(fh);
  return true;
}

void sv_file_free(StringView *file_content) {
  free((char *)file_content->data);
}

bool sb_resize(StringBuffer *sb, size_t new_cap) {
  sb->data = realloc((char *)sb->data, sizeof(char) * new_cap);
  if (sb->data == NULL) {
    return false;
  }
  sb->cap = new_cap;
  return true;
}

StringBuffer *sb_new() {
  StringBuffer *sb = (StringBuffer *)malloc(sizeof(StringBuffer));
  *sb = (StringBuffer){
      .data = (char *)malloc(sizeof(char) * SB_INITIAL_CAP),
      .cap = SB_INITIAL_CAP,
      .len = 0,
  };
  return sb;
}

StringBuffer *sb_new_with_custom_cap(size_t cap) {
  StringBuffer *sb = (StringBuffer *)malloc(sizeof(StringBuffer));
  *sb = (StringBuffer){
      .data = (char *)malloc(sizeof(char) * cap),
      .cap = cap,
      .len = 0,
  };
  return sb;
}

StringBuffer *sb_new_from_sv(StringView view) {
  StringBuffer *sb = sb_new_with_custom_cap(view.len);
  *sb = (StringBuffer){
      .data = sv_dup(view),
      .len = view.len,
      .cap = view.len,
  };

  return sb;
}

void sb_print(StringBuffer *sb) {
  printf("\"");
  for (size_t i = 0; i < sb->len; ++i) {
    printf("%c", sb->data[i]);
  }
  printf("\"");
  printf("\n");
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

bool sb_append_sb(StringBuffer *sb, StringBuffer *append) {
  if (sb->len + append->len >= sb->cap) {
    if (!sb_resize(sb, sb->cap * 2)) {
      return false;
    }
  }
  memcpy((char *)sb->data + sb->len, append->data, append->len);
  sb->len += sb->len;
  ((char *)sb->data)[sb->len] = '\0';
  return true;
}

bool sb_append(StringBuffer *sb, StringView sv) {
  if (sb->len + sv.len >= sb->cap) {
    if (!sb_resize(sb, sb->cap * 2)) {
      return false;
    }
  }
  memcpy((char *)sb->data + sb->len, sv.data, sv.len);
  sb->len += sv.len;
  ((char *)sb->data)[sb->len] = '\0';
  return true;
}

bool sb_append_char(StringBuffer *sb, char ch) {
  if (sb->len + 1 >= sb->cap) {
    if (!sb_resize(sb, sb->cap * 2)) {
      return false;
    }
  }
  ((char *)sb->data)[sb->len] = ch;
  sb->len += 1;
  ((char *)sb->data)[sb->len] = '\0';
  return true;
}

bool sb_insert(StringBuffer *sb, size_t idx, StringView sv) {
  if (idx > sb->len) {
    return false;
  }
  if (sb->len + sv.len >= sb->cap) {
    if (!sb_resize(sb, sb->cap * 2 + sv.len)) {
      return false;
    }
  }
  memmove((char *)sb->data + sb->len - sv.len, sb->data + idx, sb->len - idx);
  memcpy((char *)sb->data + idx, sv.data, sv.len);
  sb->len += sv.len;
  ((char *)sb->data)[sb->len] = '\0';

  return true;
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
