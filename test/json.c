#include <assert.h>

#include "../src/json.h"

void show(StringView key, Json *value) { sv_print(&key); }

void test_json_free() {
  JsonArray *array = json_array_new();
  json_array_free(array);
  printf("free array\n");

  JsonObject *obj = json_object_new(4);
  json_object_free(obj);
  printf("free object\n");
}

void test_json_object() {
  JsonObject *obj = json_object_new(4);
  Json *json = json_new();
  json->type = JSON_STRING;
  StringView key = sv_new_from_cstr("key");
  StringView value = sv_new_from_cstr("value");
  json->string = value;
  json_object_set(obj, key, json);

  Json *target = json_object_get(obj, key);
  StringView str = target->string;
  assert(sv_compare(str, value) && "not equal");

  json_free(json);
}

void test_json_parse() {
  StringView input = sv_new_from_cstr(
      "[\"okej\",\"nie\",\"ok\",{\"key\":\"value\"},"
      "[\"value_in_array\"],123, true, false, null, 234.2, 2, -12.2]");

  Json *json = json_parse(&input);

  assert(json->type == JSON_ARRAY && "should be array type");

  StringView a = sv_new_from_cstr("okej");
  assert(sv_compare(json->array->items[0]->string, a) && "should a be equal");

  StringView b = sv_new_from_cstr("nie");
  assert(sv_compare(json->array->items[1]->string, b) && "should b be equal");

  StringView c = sv_new_from_cstr("ok");
  assert(sv_compare(json->array->items[2]->string, c) && "should c be equal");

  StringView d_k = sv_new_from_cstr("key");
  StringView d_v = sv_new_from_cstr("value");
  Json *d_get_v = json_object_get(json->array->items[3]->object, d_k);
  assert(d_get_v->type == JSON_STRING && "should d_get_v be JSON_STRING");
  assert(sv_compare(d_get_v->string, d_v) && "should d_get_v be equal");

  StringView e = sv_new_from_cstr("value_in_array");
  assert(json->array->items[4]->type == JSON_ARRAY && "should e be JSON_ARRAY");
  assert(sv_compare(e, json->array->items[4]->array->items[0]->string) &&
         "should e be equal");

  json_free(json);
}

void test_json() {
  test_json_free();
  test_json_object();
  test_json_parse();
}
