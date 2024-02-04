#include "string_utils.h"

#ifndef _JSON_H
#define _JSON_H

#define JSON_END_OF_INPUT '\0'
#define JSON_ARRAY_CAP_INIT 3

#define JSON_OBJECT_SIZE_INIT 16

typedef struct {
  size_t line;
  size_t offset;
} Location;

typedef struct {
  StringView *input;
  Location location;
  size_t idx;
  size_t read_idx;
  char ch;
} Lexer;

typedef enum {
  JSON_STRING,
  JSON_INT,
  JSON_DOUBLE,
  JSON_ARRAY,
  JSON_OBJECT,
  JSON_NULL,
  JSON_TRUE,
  JSON_FALSE,
} JsonType;

typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;

typedef struct Json {
  JsonType type;
  JsonArray *array;
  JsonObject *object;
  StringView string;
  int num_integer;
  double num_double;
} Json;

typedef struct JsonArray {
  Json **items;
  size_t len;
  size_t cap;
} JsonArray;

typedef struct JsonObjectPair {
  struct JsonObjectPair *next;
  StringView key;
  Json *value;
} JsonObjectPair;

typedef struct JsonObject {
  JsonObjectPair **buckets;
  size_t size;
} JsonObject;

Lexer lexer_new(StringView *input);

Json *json_parse(StringView *input);
Json *json_new();

void json_free(Json *json);

StringView json_stringify(Json *json);

Json *json_parse_string(Lexer *lexer);
Json *json_parse_value(Lexer *lexer);
Json *json_parse_array(Lexer *lexer);
Json *json_parse_object(Lexer *lexer);

JsonArray *json_array_new();
void json_array_append(JsonArray *a, Json *item);
void json_array_resize(JsonArray *a, size_t new_cap);
void json_array_free(JsonArray *a);

JsonObject *json_object_new(size_t size);
void json_object_set(JsonObject *o, StringView key, Json *value);
Json *json_object_get(JsonObject *o, StringView key);
size_t json_object_hash(StringView key);
void json_object_free(JsonObject *o);
void json_object_foreach(JsonObject *o,
                         void (*callback)(StringView key, Json *value));
void json_print(Json *json);
void json_object_free(JsonObject *o);

#endif // _JSON_H
