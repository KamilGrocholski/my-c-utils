#include <assert.h>
#include <stdbool.h>

#include "_all.h"

#include "../src/string_utils.h"

bool test_sv_find(StringView haystack, StringView needle, ssize_t expected) {
  return sv_find(haystack, needle) == expected;
}

bool test_sv_is_empty(StringView sv, bool expected) {
  return sv_is_empty(sv) == expected;
}

bool test_sv_compare(StringView a, StringView b, bool expected) {
  return sv_compare(a, b) == expected;
}

bool test_sv_contains(StringView haystack, StringView needle, bool expected) {
  return sv_contains(haystack, needle) == expected;
}

bool test_sv_starts_with(StringView src, StringView starts_with,
                         bool expected) {
  return sv_starts_with(src, starts_with) == expected;
}

bool test_sv_ends_with(StringView src, StringView ends_with, bool expected) {
  return sv_ends_with(src, ends_with) == expected;
}

bool test_sv_pop_first_split_by(StringView *src, StringView split_by,
                                StringView *expected_pops) {
  size_t pops_count = 0;
  StringView pop;
  while (!sv_is_empty(*src)) {
    pop = sv_pop_first_split_by(src, split_by);
    if (!sv_compare(expected_pops[pops_count], pop))
      return false;
    pops_count++;
  }
  return true;
}

bool test_sv_is_valid_cstr(StringView src, bool expected) {
  return sv_is_valid_cstr(src) == expected;
}

bool test_sv_sub(StringView src, size_t idx, size_t count,
                 StringView expected) {
  return sv_compare(sv_sub(src, idx, count), expected);
}

void test_sv_trim() {
  StringView sv = sv_new_from_cstr("    ABCD    ");
  assert(sv_compare(sv_trim_left(sv), sv_new_from_cstr("ABCD    ")) &&
         "should sv trim left");
  assert(sv_compare(sv_trim_right(sv), sv_new_from_cstr("    ABCD")) &&
         "should sv trim right");
  assert(sv_compare(sv_trim(sv), sv_new_from_cstr("ABCD")) && "should sv trim");
}

bool test_sb_new() { return sb_new()->data != NULL; }
bool test_sb_clear() {
  StringBuffer *sb = sb_new_from_sv(sv_new_from_cstr("AB"));
  sb_clear(sb);
  return sb->len == 0 && sb->data[sb->len] == '\0';
}

bool test_sb_compare_sv(StringBuffer *sb, StringView sv, bool expected) {
  return sb_compare_sv(sb, sv) == expected;
}

bool test_sb_compare(StringBuffer *a, StringBuffer *b, bool expected) {
  return sb_compare(a, b) == expected;
}

bool test_sb_append(StringBuffer *src, StringView append, StringView expected) {
  sb_append(src, append);
  return sb_compare_sv(src, expected);
}

bool test_sb_insert(StringBuffer *src, StringView append, size_t idx,
                    StringView expected) {
  sb_insert(src, idx, append);
  return sb_compare_sv(src, expected);
}

bool test_sb_remove(StringBuffer *src, size_t idx, size_t count,
                    StringView expected) {
  sb_remove(src, idx, count);
  return sb_compare_sv(src, expected);
}

void test_string_utils() {
  // SV

  assert(test_sv_find(sv_new_from_cstr("ABCD"), sv_new_from_cstr("AB"), 0) &&
         "should find at 0");
  assert(test_sv_find(sv_new_from_cstr("ABCD"), sv_new_from_cstr("D"), 3) &&
         "should find last");
  assert(test_sv_find(sv_new_from_cstr("ABCD"), sv_new_from_cstr("BC"), 1) &&
         "should find inside");
  assert(test_sv_find(sv_new_from_cstr("ABCD"), sv_new_from_cstr("E"), -1) &&
         "should not find");

  assert(
      test_sv_contains(sv_new_from_cstr("ABCD"), sv_new_from_cstr("D"), true) &&
      "should contain");
  assert(test_sv_contains(sv_new_from_cstr("ABCD"), sv_new_from_cstr("E"),
                          false) &&
         "should not contain");

  assert(test_sv_is_empty(sv_new_from_cstr(""), true) && "should be empty");
  assert(test_sv_is_empty(sv_new_from_cstr("a"), false) &&
         "should not be empty");

  assert(test_sv_compare(sv_new_from_cstr(""), sv_new_from_cstr(""), true) &&
         "should compare both empty true");
  assert(
      test_sv_compare(sv_new_from_cstr("aa"), sv_new_from_cstr("aa"), true) &&
      "should compare true");
  assert(
      test_sv_compare(sv_new_from_cstr("aa"), sv_new_from_cstr("ab"), false) &&
      "should compare false");

  assert(
      test_sv_starts_with(sv_new_from_cstr(""), sv_new_from_cstr(""), true) &&
      "should starts_with both empty true");
  assert(test_sv_starts_with(sv_new_from_cstr("AA"), sv_new_from_cstr("AA"),
                             true) &&
         "should starts_with");
  assert(test_sv_starts_with(sv_new_from_cstr("AB"), sv_new_from_cstr("A"),
                             true) &&
         "should starts_with not equal");
  assert(test_sv_starts_with(sv_new_from_cstr("AB"), sv_new_from_cstr("B"),
                             false) &&
         "should not starts_with");

  assert(test_sv_ends_with(sv_new_from_cstr(""), sv_new_from_cstr(""), true) &&
         "should ends_with both empty true");
  assert(
      test_sv_ends_with(sv_new_from_cstr("AA"), sv_new_from_cstr("AA"), true) &&
      "should ends_with");
  assert(
      test_sv_ends_with(sv_new_from_cstr("AB"), sv_new_from_cstr("B"), true) &&
      "should ends_with not equal");
  assert(
      test_sv_ends_with(sv_new_from_cstr("AB"), sv_new_from_cstr("A"), false) &&
      "should not ends_with");

  StringView expected_pops[] = {
      sv_new_from_cstr("AB"),
      sv_new_from_cstr("CD"),
      sv_new_from_cstr("EF"),
      sv_new_from_cstr("GH"),
  };
  StringView pop_src = sv_new_from_cstr("AB/CD/EF/GH");
  assert(test_sv_pop_first_split_by(&pop_src, sv_new_from_cstr("/"),
                                    expected_pops) &&
         "should pop all");

  assert(test_sv_is_valid_cstr(sv_new_from_cstr("okej"), true) &&
         "should be valid cstr");

  assert(test_sv_sub(sv_new_from_cstr("AB/CD/EF"), 3, 1000,
                     sv_new_from_cstr("CD/EF")) &&
         "should sub overflow count");

  assert(
      test_sv_sub(sv_new_from_cstr("AB/CD/EF"), 3, 2, sv_new_from_cstr("CD")) &&
      "should sub inrange count");

  test_sv_trim();

  // SB

  assert(test_sb_new() && "should sb_new");
  assert(test_sb_clear() && "should sb_clear");

  assert(test_sb_compare(sb_new_from_sv(sv_new_from_cstr("okej")),
                         sb_new_from_sv(sv_new_from_cstr("okej")), true) &&
         "should compare sb");

  assert(test_sb_compare(sb_new_from_sv(sv_new_from_cstr("AA")),
                         sb_new_from_sv(sv_new_from_cstr("BB")), false) &&
         "should not compare sb");

  assert(test_sb_compare_sv(sb_new_from_sv(sv_new_from_cstr("AA")),
                            (sv_new_from_cstr("AA")), true) &&
         "should compare sb to sv");
  assert(test_sb_compare_sv(sb_new_from_sv(sv_new_from_cstr("AA")),
                            (sv_new_from_cstr("BB")), false) &&
         "should not compare sb to sv");

  assert(test_sb_append(sb_new(), sv_new_from_cstr("BB"),
                        sv_new_from_cstr("BB")) &&
         "should append sv to empty sb");
  assert(test_sb_append(sb_new_from_sv(sv_new_from_cstr("AA")),
                        sv_new_from_cstr("BB"), sv_new_from_cstr("AABB")) &&
         "should append sv to non empty sb");

  assert(test_sb_insert(sb_new(), sv_new_from_cstr("BB"), 0,
                        sv_new_from_cstr("BB")) &&
         "should insert sv to empty sb with overflow index");
  assert(test_sb_insert(sb_new_from_sv(sv_new_from_cstr("AA")),
                        sv_new_from_cstr("BB"), 2, sv_new_from_cstr("AABB")) &&
         "should insert sv to non empty sb");

  assert(test_sb_remove(sb_new_from_sv(sv_new_from_cstr("AABBCC")), 2, 2,
                        sv_new_from_cstr("AACC")) &&
         "should remove from sb");
}
