#include <assert.h>
#include <stdbool.h>

#include "../src/string_utils.h"
#include "../src/uri.h"

bool test_uri_parse() {
  StringView raw = sv_new_from_cstr(
      "https://www.example.com/path/to/resource?query=123#fragment");
  UriComponents components;
  uri_parse(raw, &components);
  assert(sv_compare(components.scheme, sv_new_from_cstr("https")) &&
         "should parse scheme");

  assert(sv_compare(components.host, sv_new_from_cstr("www.example.com")) &&
         "should parse host");

  assert(sv_compare(components.path, sv_new_from_cstr("path/to/resource")) &&
         "should parse path");

  assert(sv_compare(components.query, sv_new_from_cstr("query=123")) &&
         "should parse query");

  assert(sv_compare(components.fragment, sv_new_from_cstr("fragment")) &&
         "should parse fragment");

  return true;
}

void test_uri_components_join(UriComponents *components, StringView expected) {
  assert(sb_compare_sv(uri_components_join(components), expected) &&
         "should join components");
}

void test_uri() {
  assert(test_uri_parse() && "should parse uri");

  UriComponents components = {
      .scheme = sv_new_from_cstr("https"),
      .host = sv_new_from_cstr("www.example.com"),
      .path = sv_new_from_cstr("path/to/resource"),
      .query = sv_new_from_cstr("query=123"),
      .fragment = sv_new_from_cstr("fragment"),
  };
  test_uri_components_join(
      &components,
      sv_new_from_cstr(
          "https://www.example.com/path/to/resource?query=123#fragment"));
}
