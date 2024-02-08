#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "../src/json.h"

void test_json_compare() {
  Json *json_string1 = json_new_string("Hello");
  Json *json_string2 = json_new_string("Hello");
  Json *json_int1 = json_new_int(42);
  Json *json_int2 = json_new_int(42);
  Json *json_double1 = json_new_double(3.14);
  Json *json_double2 = json_new_double(3.14);
  Json *json_array1 = json_new();
  json_array1->type = JSON_ARRAY;
  json_array1->array = json_array_new();
  Json *json_array2 = json_new();
  json_array2->array = json_array_new();
  json_array2->type = JSON_ARRAY;

  assert(sb_compare(json_string1->string, json_string2->string) &&
         "should compare string");
  assert(json_double1->num_double == json_double2->num_double &&
         "should compare double");
  assert(json_int1->num_integer == json_int2->num_integer &&
         "should compare int");
  assert(json_array1->array->len == json_array2->array->len &&
         "should compare array");

  json_free(json_string1);
  json_free(json_string2);
  json_free(json_int1);
  json_free(json_int2);
  json_free(json_double1);
  json_free(json_double2);
  json_free(json_array1);
  json_free(json_array2);
}

void test_json_parse_file() {
  Json *json = json_new();
  assert(json_parse_file("test.json", json) && "should parse from file");

  JsonObject *first = json->array->items[0]->object;
  JsonObject *second = json->array->items[1]->object;

  assert(sb_compare_sv(json_object_get(first, sv_new_from_cstr("name"))->string,
                       sv_new_from_cstr("John")) &&
         "first name");
  assert(25 == json_object_get(first, sv_new_from_cstr("age"))->num_integer &&
         "first age");
  assert(JSON_TRUE ==
             json_object_get(first, sv_new_from_cstr("isStudent"))->type &&
         "first isStudent");

  assert(
      sb_compare_sv(json_object_get(second, sv_new_from_cstr("name"))->string,
                    sv_new_from_cstr("Alice")) &&
      "second name");
  assert(30 == json_object_get(second, sv_new_from_cstr("age"))->num_integer &&
         "second age");
  assert(JSON_FALSE ==
             json_object_get(second, sv_new_from_cstr("isStudent"))->type &&
         "second isStudent");

  json_free(json);
}

void test_json_parse_array() {
  Json *json = json_new_array();
  Json *obj = json_new_object();
  json_object_set(obj->object, sb_new_from_cstr("key1"),
                  json_new_string("value1"));
  Json *items[] = {
      json_new_string("okej"),
      json_new_int(2),
      json_new_double(4),
      json_new_array(),
      json_new_bool(true),
      json_new_null(),
      obj,
  };
  for (size_t i = 0; i < sizeof(items) / sizeof(Json *); ++i) {
    json_array_append(json->array, items[i]);
  }
  assert(sb_compare_sv(json->array->items[0]->string, sv_new("okej", 4)));
  assert(json->array->items[1]->num_integer == 2);
  assert(json->array->items[2]->num_double == 4);
  assert(json->array->items[3]->array->len == 0);
  assert(json->array->items[4]->type == JSON_TRUE);
  assert(json->array->items[5]->type == JSON_NULL);
  assert(json->array->items[6]->type == JSON_OBJECT);

  json_free(json);
}

void test_json_parse_object() {
  Json *json = json_new_object();
  json_object_set(json->object, sb_new_from_cstr("key1"),
                  json_new_string("value1"));
  json_object_set(json->object, sb_new_from_cstr("key2"),
                  json_new_string("value2"));

  Json *value1 = json_object_get(json->object, sv_new_from_cstr("key1"));
  Json *value2 = json_object_get(json->object, sv_new_from_cstr("key2"));

  assert(sb_compare_sv(value1->string, sv_new_from_cstr("value1")));
  assert(sb_compare_sv(value2->string, sv_new_from_cstr("value2")));
  json_free(json);
}

void test_json_stringify_array() {
  Json *json = json_new_array();
  json_array_append(json->array, json_new_string("okej"));
  json_array_append(json->array, json_new_null());
  json_array_append(json->array, json_new_bool(true));
  json_array_append(json->array, json_new_bool(false));
  json_array_append(json->array, json_new_int(2));
  json_array_append(json->array, json_new_double(2.234));
  json_array_append(json->array, json_new_object());
  json_array_append(json->array, json_new_array());

  StringBuffer *buff = sb_new();
  json_stringify(json, buff);

  sb_free(buff);
  json_free(json);
}

void test_json_parse_array_fail() {
  Json *json = json_new();
  json_print(json);
  StringBuffer *input = sb_new_from_cstr("[\"AB\",:]");
  assert(!json_parse(input, json) && "test_json_parse_array_fail should fail");
  sb_free(input);
  json_free(json);
}

void test_json_parse_object_fail() {
  Json *json = json_new();
  json_print(json);
  StringBuffer *input = sb_new_from_cstr("{\"key\"::");
  assert(!json_parse(input, json) && "test_json_parse_object_fail should fail");
  sb_free(input);
  json_free(json);
}

void test_json() {
  test_json_compare();

  test_json_parse_file();

  test_json_parse_object();
  test_json_parse_object_fail();

  test_json_parse_array();
  test_json_parse_array_fail();

  test_json_stringify_array();

  printf("All 'json' tests passed successfully!\n");
}
