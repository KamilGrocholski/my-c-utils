#include <assert.h>

#include "../src/json.h"

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

void test_json_from_file() {
  StringView file_content = {0};
  sv_file_read("test.json", &file_content);
  Json *json = json_parse(&file_content);

  struct User {
    char *name;
    int age;
    bool isStudent;
  };

  struct User users[] = {
      {"John", 25, true}, {"Alice", 30, false},  {"Bob", 22, true},
      {"Eve", 28, false}, {"Charlie", 24, true},
  };

  Json *expected = json_new();
  JsonArray *expected_array = json_array_new();
  expected->type = JSON_ARRAY;
  expected->array = expected_array;
  for (size_t i = 0; i < sizeof(users) / sizeof(users[0]); ++i) {
    Json *value = json_new();
    JsonObject *obj = json_object_new(3);
    value->type = JSON_OBJECT;
    value->object = obj;

    Json *name = json_new();
    name->type = JSON_STRING;
    name->string = sv_new_from_cstr(users[i].name);
    json_object_set(obj, sv_new_from_cstr("name"), name);

    Json *age = json_new();
    age->type = JSON_INT;
    age->num_integer = users[i].age;
    json_object_set(obj, sv_new_from_cstr("age"), age);

    Json *is_student = json_new();
    is_student->type = users[i].isStudent ? JSON_TRUE : JSON_FALSE;
    json_object_set(obj, sv_new_from_cstr("isStudent"), is_student);

    json_array_append(expected_array, value);
  }

  for (size_t i = 0; i < sizeof(users) / sizeof(users[0]); ++i) {
    JsonObject *user_json = json->array->items[i]->object;
    JsonObject *user_expected = expected->array->items[i]->object;

    /* printf("i: %lu\n", i); */

    Json *name_json = json_object_get(user_json, sv_new_from_cstr("name"));
    Json *name_expected =
        json_object_get(user_expected, sv_new_from_cstr("name"));
    /* printf("name_json: "); */
    /* SV_PRINT(name_json->string); */
    /* printf(", name_expected: "); */
    /* SV_PRINT(name_expected->string); */
    /* printf("\n"); */
    assert(sv_compare(name_json->string, name_expected->string) &&
           "should names be equal");

    Json *age_json = json_object_get(user_json, sv_new_from_cstr("age"));
    Json *age_expected =
        json_object_get(user_expected, sv_new_from_cstr("age"));
    /* printf("age_json: %d", age_json->num_integer); */
    /* printf(", age_expected: %d\n", age_expected->num_integer); */
    assert(age_json->num_integer == age_expected->num_integer &&
           "should age be equal");

    Json *is_student_json =
        json_object_get(user_json, sv_new_from_cstr("isStudent"));
    Json *is_student_expected =
        json_object_get(user_expected, sv_new_from_cstr("isStudent"));
    /* printf("isStudent_json: %s", */
    /*        is_student_json->type == JSON_TRUE ? "TRUE" : "FALSE"); */
    /* printf(", is_student_expected: %s", */
    /*        is_student_expected->type == JSON_TRUE ? "TRUE" : "FALSE"); */
    assert(is_student_json->type == is_student_expected->type &&
           "should isStudent be equal");

    /* printf("\n"); */
  }

  json_free(expected);
  json_free(json);
  sv_file_free(&file_content);
}

void test_json() {
  test_json_free();
  test_json_object();
  test_json_parse();
  test_json_from_file();
}
