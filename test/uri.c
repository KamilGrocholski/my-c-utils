#include <assert.h>
#include <stdbool.h>

#include "../src/string_utils.h"
#include "../src/uri.h"

bool test_uri_parse() {
  StringView raw =
      sv_new_from_cstr("https://www.example.com/path/to/resource?query=123");
  UriComponents components = {0};
  uri_parse(raw, &components);

  assert(sv_compare(components.scheme, sv_new_from_cstr("https")) &&
         "should parse scheme");

  assert(sv_compare(components.host, sv_new_from_cstr("www.example.com")) &&
         "should parse host");

  assert(sv_compare(components.path, sv_new_from_cstr("path/to/resource")) &&
         "should parse path");

  assert(sv_compare(components.query, sv_new_from_cstr("query=123")) &&
         "should parse query");

  assert(sv_compare(components.fragment, sv_new_from_cstr("")) &&
         "should parse fragment");

  return true;
}

void test_uri_components_join(UriComponents *components, StringView expected) {
  StringBuffer *joined = uri_components_join(components);
  assert(sb_compare_sv(joined, expected) && "should join components");
  sb_free(joined);
}

void callback(StringView key, StringView value) {
  sv_print(&key);
  sv_print(&value);
}

void test_uri_query_parse() {
  UriQueryPairs *qp = uri_query_new();
  StringView query = sv_new_from_cstr("key1=11&key2=22&key3aa=33aa");
  uri_query_parse(qp, query);

  StringView key1_value = uri_query_get(qp, sv_new_from_cstr("key1"));
  StringView key2_value = uri_query_get(qp, sv_new_from_cstr("key2"));
  StringView key3_value = uri_query_get(qp, sv_new_from_cstr("key3aa"));
  assert(sv_compare(key1_value, sv_new_from_cstr("11")) && "should get key1");
  assert(sv_compare(key2_value, sv_new_from_cstr("22")) && "should get key2");
  assert(sv_compare(key3_value, sv_new_from_cstr("33aa")) && "should get key3");
  uri_query_free(qp);
}

void test_uri_query_methods() {
  UriQueryPairs *qp = uri_query_new();
  UriQueryPair p1 = {
      .key = sv_new_from_cstr("key1"),
      .value = sv_new_from_cstr("value1"),
  };
  UriQueryPair p2 = {
      .key = sv_new_from_cstr("key2"),
      .value = sv_new_from_cstr("value2"),
  };
  uri_query_set(qp, p1.key, p1.value);
  uri_query_set(qp, p2.key, p2.value);
  assert(uri_query_has(qp, p1.key) && "should get p1 bool");
  assert(sv_compare(uri_query_get(qp, p1.key), p1.value) && "should get p1");
  assert(sv_compare(uri_query_get(qp, p2.key), p2.value) && "should get p2");
  assert(sv_compare(uri_query_remove(qp, p2.key), p2.value) &&
         "should remove p2");
  uri_query_free(qp);
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

  test_uri_query_parse();

  printf("All 'uri' tests passed successfully!\n");
}
