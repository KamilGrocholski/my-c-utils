#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _STRING_UTILS_H
#define _STRING_UTILS_H

#define SB_INITIAL_CAP 64
#define SB_INT_CAP 20
#define SB_DOUBLE_CAP 40

#define SV_FMT "%.*s"
#define SB_FMT "%.*s"
#define SV_PRINT(sv) printf(SV_FMT, (int)(sv.len), sv.data)
#define SB_PRINT(sb) printf(SB_FMT, (int)(sb->len), sb->data)
#define SV_NEW_FROM_CSTR(cstr)                                                 \
  (StringView) { .data = cstr, .len = sizeof(cstr), }

typedef struct {
  const char *data;
  size_t len;
} StringView;

typedef struct {
  const char *data;
  size_t cap;
  size_t len;
} StringBuffer;

// SV

const char *cstr(const char *literal);

StringView sv_new(const char *data, size_t len);

StringView sv_new_from_cstr(const char *data);

StringView sv_pop_first_split_by(StringView *src, StringView split_by);

void sv_print(StringView *sv);

ssize_t sv_find(StringView haystack, StringView needle);

bool sv_is_empty(StringView sv);

bool sv_starts_with(StringView sv, StringView starts_with);

bool sv_ends_with(StringView sv, StringView ends_with);

bool sv_contains(StringView haystack, StringView needle);

bool sv_compare(StringView a, StringView b);

bool sv_compare_sb(StringView sv, StringBuffer *sb);

StringView sv_sub(StringView sv, size_t position, size_t count);

StringView sv_trim_left(StringView sv);

StringView sv_trim_right(StringView sv);

StringView sv_trim(StringView sv);

char *sv_dup(StringView sv);

// SB

void sb_resize(StringBuffer *sb, size_t new_cap);

StringBuffer *sb_new();

StringBuffer *sb_new_with_custom_cap(size_t cap);

StringBuffer *sb_new_from_sv(StringView view);

StringBuffer *sb_new_from_cstr(const char *data);

StringBuffer *sb_sub(StringBuffer *sb, size_t start, size_t end);

void sb_print(StringBuffer *sb);

void sb_free(StringBuffer *sb);

void sb_clear(StringBuffer *sb);

bool sb_compare(StringBuffer *a, StringBuffer *b);

bool sb_compare_sv(StringBuffer *sb, StringView sv);

bool sb_is_valid_cstr(StringBuffer *sb);

bool sb_is_empty(StringBuffer *sb);

void sb_append(StringBuffer *sb, StringView sv);

void sb_append_char(StringBuffer *sb, char ch);

void sb_append_sb(StringBuffer *sb, StringBuffer *append);

void sb_insert(StringBuffer *sb, size_t idx, StringView sv);

void sb_remove(StringBuffer *sb, size_t idx, size_t count);

bool sb_file_read(const char *filename, StringBuffer *dest);

#endif // _STRING_BUFFER_H
