#include <assert.h>
#include <stdio.h>

#include "../src/string_utils.h"

void test_sv_starts_with() {
  StringView sv1 = sv_new_from_cstr("hello world");
  StringView sv2 = sv_new_from_cstr("hello");
  StringView sv3 = sv_new_from_cstr("world");

  assert(sv_starts_with(sv1, sv2) && "test_sv_starts_with failed");
  assert(!sv_starts_with(sv1, sv3) && "test_sv_starts_with failed");
}

void test_sv_ends_with() {
  StringView sv1 = sv_new_from_cstr("hello world");
  StringView sv2 = sv_new_from_cstr("world");
  StringView sv3 = sv_new_from_cstr("hello");

  assert(sv_ends_with(sv1, sv2) && "test_sv_ends_with failed");
  assert(!sv_ends_with(sv1, sv3) && "test_sv_ends_with failed");
}

void test_sv_trim() {
  StringView sv1 = sv_new_from_cstr("  hello world  ");
  StringView sv2 = sv_new_from_cstr("hello world");

  assert(sv_compare(sv_trim(sv1), sv2) && "test_sv_trim failed");
}

void test_sv_find() {
  StringView sv1 = sv_new_from_cstr("hello world");
  StringView sv2 = sv_new_from_cstr("world");

  assert(sv_find(sv1, sv2) == 6 && "test_sv_find failed");
}

void test_sv_contains() {
  StringView sv1 = sv_new_from_cstr("hello world");
  StringView sv2 = sv_new_from_cstr("world");
  StringView sv3 = sv_new_from_cstr("foo");

  assert(sv_contains(sv1, sv2) && "test_sv_contains failed");
  assert(!sv_contains(sv1, sv3) && "test_sv_contains failed");
}

void test_sv_compare() {
  StringView sv1 = sv_new_from_cstr("hello");
  StringView sv2 = sv_new_from_cstr("hello");
  StringView sv3 = sv_new_from_cstr("world");

  assert(sv_compare(sv1, sv2) && "test_sv_compare failed");
  assert(!sv_compare(sv1, sv3) && "test_sv_compare failed");
}

void test_sv_trim_left() {
  StringView sv1 = sv_new_from_cstr("  hello");
  StringView sv2 = sv_new_from_cstr("hello");
  StringView sv3 = sv_new_from_cstr(" ");
  StringView sv4 = sv_new_from_cstr("");

  assert(sv_compare(sv_trim_left(sv1), sv2) && "test_sv_trim_left failed");
  assert(sv_compare(sv_trim_left(sv3), sv4) &&
         "test_sv_trim_left failed empty");
}

void test_sv_trim_right() {
  StringView sv1 = sv_new_from_cstr("hello  ");
  StringView sv2 = sv_new_from_cstr("hello");

  StringView sv3 = sv_new_from_cstr(" ");
  StringView sv4 = sv_new_from_cstr("");

  assert(sv_compare(sv_trim_right(sv1), sv2) && "test_sv_trim_right failed");
  assert(sv_compare(sv_trim_right(sv3), sv4) &&
         "test_sv_trim_right failed empty");
}

void test_sv_is_empty() {
  StringView sv1 = sv_new_from_cstr("");
  StringView sv2 = sv_new_from_cstr("hello");

  assert(sv_is_empty(sv1) && "test_sv_is_empty failed");
  assert(!sv_is_empty(sv2) && "test_sv_is_empty failed");
}

void test_sb_append() {
  StringBuffer *sb = sb_new();
  StringView sv1 = sv_new_from_cstr("hello");
  StringView sv2 = sv_new_from_cstr(" world");

  sb_append(sb, sv1);
  sb_append(sb, sv2);

  assert(sb->len == 11 && "test_sb_append failed len");
  assert(sb_compare_sv(sb, sv_new_from_cstr("hello world")) &&
         "test_sb_append failed compare");

  sb_free(sb);
}

void test_sb_append_sb() {
  StringBuffer *sb1 = sb_new_from_cstr("hello");
  StringBuffer *sb2 = sb_new_from_cstr(" world");

  sb_append_sb(sb1, sb2);

  assert(sb1->len == 11 && "test_sb_append_sb failed");
  assert(sb_compare_sv(sb1, sv_new_from_cstr("hello world")) &&
         "test_sb_append_sb failed");

  sb_free(sb1);
  sb_free(sb2);
}

void test_sb_insert() {
  StringBuffer *sb = sb_new_from_cstr("hello");

  sb_insert(sb, 5, sv_new_from_cstr(" world"));

  assert(sb->len == 11 && "test_sb_insert failed");
  assert(sb_compare_sv(sb, sv_new_from_cstr("hello world")) &&
         "test_sb_insert failed");

  sb_free(sb);
}

void test_sb_sub() {
  StringBuffer *sb = sb_new_from_cstr("ABCDE");
  StringBuffer *sub = sb_sub(sb, 1, 3);

  assert(sub->len == 3 && "test_sb_sub failed len");
  assert(sb_compare_sv(sub, sv_new_from_cstr("BCD")) &&
         "test_sb_sub failed compare");
  sb_free(sb);
}

void test_sb_clear() {
  StringBuffer *sb = sb_new_from_cstr("hello world");

  sb_clear(sb);

  assert(sb->len == 0 && "test_sb_clear failed");
  assert(sb_is_empty(sb) && "test_sb_clear failed");

  sb_free(sb);
}

void test_sb_remove() {
  StringBuffer *sb = sb_new_from_cstr("hello world");

  sb_remove(sb, 5, 6);

  assert(sb->len == 5 && "test_sb_remove failed");
  assert(sb_compare_sv(sb, sv_new_from_cstr("hello")) &&
         "test_sb_remove failed");

  sb_free(sb);
}

void test_string_utils() {
  test_sv_starts_with();
  test_sv_ends_with();
  test_sv_trim();
  test_sv_find();
  test_sv_contains();
  test_sv_compare();
  test_sv_trim_left();
  test_sv_trim_right();
  test_sv_is_empty();

  test_sb_append();
  test_sb_insert();
  test_sb_append_sb();
  test_sb_sub();
  test_sb_clear();
  test_sb_remove();

  printf("All 'string_utils' tests passed!\n");
}
