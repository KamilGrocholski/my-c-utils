#include <ctype.h>
#include <stdlib.h>

#include "json.h"
#include "logger.h"

// TODO
// https://www.crockford.com/mckeeman.html
// correct string parsing
// numbers add: errors, +/-, hex
// assert last char is valid
// CD json_stringify

Lexer lexer_new(StringBuffer *input) {
  return (Lexer){
      .input = input,
      .ch = JSON_END_OF_INPUT,
      .idx = 0,
      .read_idx = 0,
      .location =
          (Location){
              .line = 1,
              .offset = 0,
          },
  };
}

void lexer_advance(Lexer *lexer) {
  if (lexer->read_idx >= lexer->input->len) {
    lexer->ch = JSON_END_OF_INPUT;
  } else {
    if (lexer->ch == '\n') {
      lexer->location.line++;
      lexer->location.offset = 0;
    } else {
      lexer->location.offset++;
    }
    lexer->ch = lexer->input->data[lexer->read_idx];
  }
  lexer->idx = lexer->read_idx;
  lexer->read_idx++;
}

void lexer_skip_whitespace(Lexer *lexer) {
  while (lexer->ch == ' ' || lexer->ch == '\r' || lexer->ch == '\t' ||
         lexer->ch == '\n') {
    lexer_advance(lexer);
  }
}

bool lexer_eat(Lexer *lexer, char ch) {
  if (lexer->ch != ch) {
    logger_log(LOG_ERROR,
               "JSON_PARSE expected '%c' got '%c' at line %lu on offset %lu",
               ch, lexer->ch, lexer->location.line, lexer->location.offset);
    return false;
  }
  lexer_advance(lexer);
  return true;
}

StringBuffer *lexer_read_integer(Lexer *lexer) {
  size_t start = lexer->idx;
  while (isdigit(lexer->ch)) {
    lexer_advance(lexer);
  }
  size_t end = lexer->idx;
  return sb_sub(lexer->input, start, end - 1);
}

bool is_alpha_lowercase(char ch) { return ch >= 'a' && ch <= 'z'; }

StringBuffer *lexer_read_ident(Lexer *lexer) {
  size_t start = lexer->idx;
  while (is_alpha_lowercase(lexer->ch)) {
    lexer_advance(lexer);
  }
  size_t end = lexer->idx;
  return sb_sub(lexer->input, start, end - 1);
}

bool is_letter(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

StringBuffer *lexer_read_name(Lexer *lexer) {
  if (!lexer_eat(lexer, '"')) {
    return NULL;
  }
  size_t start = lexer->idx;
  while (is_letter(lexer->ch)) {
    lexer_advance(lexer);
  }
  size_t end = lexer->idx;
  if (!lexer_eat(lexer, '"')) {
    return NULL;
  }
  return sb_sub(lexer->input, start, end - 1);
}

StringBuffer *lexer_read_string(Lexer *lexer) {
  if (!lexer_eat(lexer, '"')) {
    return NULL;
  }
  size_t start = lexer->idx;
  while (lexer->ch != '"' && lexer->ch != JSON_END_OF_INPUT) {
    lexer_advance(lexer);
  }
  size_t end = lexer->idx;
  if (!lexer_eat(lexer, '"')) {
    return NULL;
  }
  return sb_sub(lexer->input, start, end - 1);
}

Json *json_new() {
  Json *json = malloc(sizeof(Json));
  if (json == NULL) {
    logger_log(LOG_FATAL, "json_new malloc err");
  }
  json->type = JSON_EMPTY;
  return json;
}

void json_free(Json *json) {
  if (json == NULL) {
    return;
  }
  switch (json->type) {
  case JSON_ARRAY:
    json_array_free(json->array);
    break;
  case JSON_OBJECT:
    json_object_free(json->object);
    break;
  case JSON_STRING:
    sb_free(json->string);
    break;
  case JSON_INT:
  case JSON_DOUBLE:
  case JSON_TRUE:
  case JSON_FALSE:
  case JSON_NULL:
  case JSON_EMPTY:
    break;
  default:
    logger_log(LOG_FATAL, "json_free invalid json type");
  }
  free(json);
}

bool json_parse(StringBuffer *input, Json *dest) {
  Lexer lexer = lexer_new(input);
  lexer_advance(&lexer);

  bool is_success = json_parse_value(&lexer, dest);

  return is_success;
}

bool json_parse_value(Lexer *lexer, Json *dest) {
  lexer_skip_whitespace(lexer);
  switch (lexer->ch) {
  case '"':
    return json_parse_string(lexer, dest);
  case '[':
    return json_parse_array(lexer, dest);
  case '{':
    return json_parse_object(lexer, dest);
  default: {
    if (isdigit(lexer->ch) || lexer->ch == '-') {
      size_t start = lexer->idx;
      lexer_advance(lexer);
      while (isdigit(lexer->ch)) {
        lexer_advance(lexer);
      }
      if (lexer->ch == '.') {
        if (!lexer_eat(lexer, '.')) {
          return false;
        }
        while (isdigit(lexer->ch)) {
          lexer_advance(lexer);
        }
        dest->type = JSON_DOUBLE;
        StringBuffer *double_str = sb_sub(lexer->input, start, lexer->idx);
        dest->num_double = atof(double_str->data);
        sb_free(double_str);
      } else {
        dest->type = JSON_INT;
        StringBuffer *int_str = sb_sub(lexer->input, start, lexer->idx);
        dest->num_integer = atoi(int_str->data);
        sb_free(int_str);
      }
      return true;
    } else if (is_alpha_lowercase(lexer->ch)) {
      StringBuffer *ident = lexer_read_ident(lexer);
      if (sb_compare_sv(ident, sv_new("null", 4))) {
        dest->type = JSON_NULL;
        sb_free(ident);
        return true;
      } else if (sb_compare_sv(ident, sv_new("true", 4))) {
        dest->type = JSON_TRUE;
        sb_free(ident);
        return true;
      } else if (sb_compare_sv(ident, sv_new("false", 5))) {
        dest->type = JSON_FALSE;
        sb_free(ident);
        return true;
      } else {
        sb_free(ident);
        logger_log(
            LOG_ERROR,
            "JSON_PARSE invalid keyword '%.*s' at line %lu on offset %lu",
            ident->len, ident->data, lexer->location.line,
            lexer->location.offset);
        return false;
      }
    }
    logger_log(LOG_ERROR,
               "JSON_PARSE unexpected ch '%c' at line %lu on offset %lu",
               lexer->ch, lexer->location.line, lexer->location.offset);
    return false;
  }
  }
  return false;
}

bool json_parse_string(Lexer *lexer, Json *dest) {
  StringBuffer *string = lexer_read_string(lexer);
  if (string == NULL) {
    return false;
  }
  *dest = (Json){
      .type = JSON_STRING,
      .string = string,
  };
  return true;
}

bool json_parse_array(Lexer *lexer, Json *dest) {
  if (!lexer_eat(lexer, '[')) {
    return false;
  }

  JsonArray *array = json_array_new();
  dest->type = JSON_ARRAY;
  dest->array = array;

  lexer_skip_whitespace(lexer);

  if (lexer->ch == ']') {
    lexer_advance(lexer);
    return true;
  }

  while (true) {
    lexer_skip_whitespace(lexer);

    Json *append = json_new();
    if (!json_parse_value(lexer, append)) {
      json_free(append);
      return false;
    }

    json_array_append(array, append);

    lexer_skip_whitespace(lexer);

    if (lexer->ch == ',') {
      lexer_advance(lexer);
    } else if (lexer->ch == ']') {
      lexer_advance(lexer);
      return true;
    } else {
      json_array_free(array);
      return false;
    }
  }

  return false;
}

void json_array_free(JsonArray *a) {
  if (a != NULL && a->items != NULL) {
    for (size_t i = 0; i < a->len; ++i) {
      json_free(a->items[i]);
    }
    free(a->items);
    free(a);
  }
}

bool json_parse_object(Lexer *lexer, Json *dest) {
  if (!lexer_eat(lexer, '{')) {
    return false;
  }

  JsonObject *object = json_object_new(JSON_OBJECT_SIZE_INIT);
  *dest = (Json){
      .type = JSON_OBJECT,
      .object = object,
  };

  lexer_skip_whitespace(lexer);
  if (lexer->ch == '}') {
    lexer_advance(lexer);
    return true;
  }

  while (true) {
    StringBuffer *name = lexer_read_name(lexer);
    if (name == NULL) {
      return false;
    }
    lexer_skip_whitespace(lexer);
    if (!lexer_eat(lexer, ':')) {
      sb_free(name);
      return false;
    }
    Json *value = json_new();
    if (value == NULL || !json_parse_value(lexer, value)) {
      sb_free(name);
      json_free(value);
      return false;
    }

    json_object_set(object, name, value);

    lexer_skip_whitespace(lexer);
    if (lexer->ch != ',') {
      break;
    }
    lexer_advance(lexer);
    lexer_skip_whitespace(lexer);
  }

  if (!lexer_eat(lexer, '}')) {
    return false;
  }

  return true;
}

JsonArray *json_array_new() {
  JsonArray *a = malloc(sizeof(JsonArray));
  if (a == NULL) {
    logger_log(LOG_FATAL, "json_array_new mem alloc err");
  }
  *a = (JsonArray){
      .len = 0,
      .cap = JSON_ARRAY_CAP_INIT,
      .items = malloc(sizeof(Json *) * JSON_ARRAY_CAP_INIT),
  };
  if (a->items == NULL) {
    logger_log(LOG_FATAL, "json_array_new->items mem alloc err");
  }
  return a;
}

void json_array_append(JsonArray *a, Json *item) {
  if (a->len >= a->cap) {
    json_array_resize(a, a->cap * 2);
  }
  a->items[a->len] = item;
  a->len++;
}

void json_array_resize(JsonArray *a, size_t new_cap) {
  a->items = realloc(a->items, new_cap * sizeof(Json *));
  if (a->items == NULL) {
    logger_log(LOG_FATAL, "json_array_resize->items mem realloc err");
  }
  a->cap = new_cap;
}

JsonObject *json_object_new(size_t size) {
  JsonObject *o = malloc(sizeof(JsonObject));
  if (o == NULL) {
    logger_log(LOG_FATAL, "json_object_new mem alloc err");
  }
  *o = (JsonObject){
      .size = size,
      .buckets = calloc(size, sizeof(JsonObjectPair *)),
  };
  if (o->buckets == NULL) {
    logger_log(LOG_FATAL, "json_object_new->buckets mem alloc err");
  }
  return o;
}

void json_object_set(JsonObject *o, StringBuffer *key, Json *value) {
  size_t hash = json_object_hash(sv_new(key->data, key->len));
  size_t idx = hash % o->size;

  JsonObjectPair *new_pair = malloc(sizeof(JsonObjectPair));
  if (new_pair == NULL) {
    logger_log(LOG_FATAL, "json_object_set mem alloc err");
  }
  *new_pair = (JsonObjectPair){
      .next = o->buckets[idx],
      .key = key,
      .value = value,
  };
  o->buckets[idx] = new_pair;
}

Json *json_object_get(JsonObject *o, StringView key) {
  size_t hash = json_object_hash(key);
  size_t idx = hash % o->size;

  JsonObjectPair *curr = o->buckets[idx];
  while (curr) {
    if (sb_compare_sv(curr->key, key)) {
      return curr->value;
    }
    curr = curr->next;
  }
  return NULL;
}

size_t json_object_hash(StringView key) {
  size_t h = 0;
  for (size_t i = 0; i < key.len; ++i) {
    h = 31 * h + key.data[i];
  }
  return h;
}

void json_object_foreach(JsonObject *o,
                         void (*callback)(StringBuffer *key, Json *value)) {
  for (size_t i = 0; i < o->size; ++i) {
    JsonObjectPair *pair = o->buckets[i];
    while (pair) {
      callback(pair->key, pair->value);
      pair = pair->next;
    }
  }
}

void json_object_free(JsonObject *o) {
  if (o == NULL) {
    return;
  }
  for (size_t i = 0; i < o->size; ++i) {
    JsonObjectPair *curr = o->buckets[i];
    while (curr != NULL) {
      JsonObjectPair *next = curr->next;
      sb_free(curr->key);
      json_free(curr->value);
      free(curr);
      curr = next;
    }
  }
  free(o->buckets);
  free(o);
}

void _json_obj_print_cb(StringBuffer *key, Json *value) {
  printf("\"%s\":", key->data);
  json_print(value);
  printf(",");
}

void json_print(Json *json) {
  switch (json->type) {
  case JSON_STRING:
    printf("\"%s\"", json->string->data);
    break;
  case JSON_ARRAY:
    printf("[");
    for (size_t i = 0; i < json->array->len; ++i) {
      if (i > 0)
        printf(",");
      json_print(json->array->items[i]);
    }
    printf("]");
    break;
  case JSON_OBJECT:
    printf("{");
    json_object_foreach(json->object, _json_obj_print_cb);
    printf("}");
    break;
  case JSON_INT:
    printf("%d", json->num_integer);
    break;
  case JSON_DOUBLE:
    printf("%f", json->num_double);
    break;
  case JSON_TRUE:
    printf("true");
    break;
  case JSON_FALSE:
    printf("false");
    break;
  case JSON_NULL:
    printf("null");
    break;
  case JSON_EMPTY:
    break;
  default:
    logger_log(LOG_FATAL, "json_print invalid type");
  }
}

bool json_object_get_int(JsonObject *o, StringView key, int **dest) {
  Json *value = json_object_get(o, key);
  if (value == NULL || value->type != JSON_INT) {
    return false;
  }
  **dest = value->num_integer;
  return true;
}

bool json_object_get_double(JsonObject *o, StringView key, double **dest) {
  Json *value = json_object_get(o, key);
  if (value == NULL || value->type != JSON_DOUBLE) {
    return false;
  }
  **dest = value->num_double;
  return true;
}

bool json_object_get_string(JsonObject *o, StringView key,
                            StringBuffer **dest) {
  Json *value = json_object_get(o, key);
  if (value == NULL || value->type != JSON_STRING) {
    *dest = NULL;
    return false;
  }
  *dest = value->string;
  return true;
}

bool json_object_get_bool(JsonObject *o, StringView key, bool **dest) {
  Json *value = json_object_get(o, key);
  if (value == NULL) {
    return false;
  }
  if (value->type == JSON_TRUE) {
    **dest = true;
    return true;
  } else if (value->type == JSON_FALSE) {
    **dest = false;
    return true;
  }

  return false;
}

bool json_object_get_array(JsonObject *o, StringView key, JsonArray **dest) {
  Json *value = json_object_get(o, key);
  if (value == NULL || value->type != JSON_ARRAY) {
    *dest = NULL;
    return false;
  }
  *dest = value->array;
  return true;
}

bool json_object_get_object(JsonObject *o, StringView key, JsonObject **dest) {
  Json *value = json_object_get(o, key);
  if (value == NULL || value->type != JSON_OBJECT) {
    *dest = NULL;
    return false;
  }
  *dest = value->object;
  return true;
}

bool json_parse_file(const char *filename, Json *json) {
  StringBuffer *content = sb_new();
  if (!sb_file_read(filename, content)) {
    sb_free(content);
    return false;
  }
  if (!json_parse(content, json)) {
    sb_free(content);
    return false;
  }
  sb_free(content);
  return true;
}

bool json_stringify_value(Json *json, StringBuffer *dest);

bool json_stringify_string(Json *json, StringBuffer *dest) {
  sb_append_char(dest, '"');
  sb_append_sb(dest, json->string);
  sb_append_char(dest, '"');
  return true;
}

bool json_stringify_array(Json *json, StringBuffer *dest) {
  sb_append_char(dest, '[');
  if (json->array->len == 0) {
    sb_append_char(dest, ']');
    return true;
  } else if (json->array->len == 1) {
    json_stringify_value(json->array->items[0], dest);
  } else {
    json_stringify_value(json->array->items[0], dest);
    for (size_t i = 1; i < json->array->len; ++i) {
      sb_append_char(dest, ',');
      json_stringify_value(json->array->items[i], dest);
    }
  }
  sb_append_char(dest, ']');
  return true;
}

bool json_stringify_object(Json *json, StringBuffer *dest) {
  sb_append_char(dest, '{');

  JsonObject *object = json->object;
  bool first = true;

  for (size_t i = 0; i < object->size; ++i) {
    JsonObjectPair *pair = object->buckets[i];
    while (pair) {
      if (!first) {
        sb_append_char(dest, ',');
      } else {
        first = false;
      }

      sb_append_char(dest, '"');
      sb_append_sb(dest, pair->key);
      sb_append_char(dest, '"');
      sb_append_char(dest, ':');
      json_stringify_value(pair->value, dest);

      pair = pair->next;
    }
  }

  sb_append_char(dest, '}');
  return true;
}

bool json_stringify_value(Json *json, StringBuffer *dest) {
  switch (json->type) {
  case JSON_STRING:
    json_stringify_string(json, dest);
    break;
  case JSON_ARRAY:
    json_stringify_array(json, dest);
    break;
  case JSON_OBJECT:
    json_stringify_object(json, dest);
    break;
  case JSON_INT: {
    char int_str[JSON_INT_STR_CAP];
    snprintf(int_str, sizeof(int_str), "%d", json->num_integer);
    sb_append(dest, sv_new_from_cstr(int_str));
  } break;
  case JSON_DOUBLE: {
    char double_str[JSON_DOUBLE_STR_CAP];
    snprintf(double_str, sizeof(double_str), "%.17g", json->num_double);
    sb_append(dest, sv_new_from_cstr(double_str));
  } break;
  case JSON_TRUE:
    sb_append(dest, sv_new_from_cstr("true"));
    break;
  case JSON_FALSE:
    sb_append(dest, sv_new_from_cstr("false"));
    break;
  case JSON_NULL:
    sb_append(dest, sv_new_from_cstr("null"));
    break;
  default:
    logger_log(LOG_FATAL, "json_stringify unreachable type");
  }
  return true;
}

bool json_stringify(Json *json, StringBuffer *dest) {
  json_stringify_value(json, dest);
  return true;
}

Json *json_new_string(const char *value) {
  Json *j = json_new();
  j->type = JSON_STRING;
  j->string = sb_new_from_cstr(value);
  return j;
}

Json *json_new_int(int value) {
  Json *j = json_new();
  j->type = JSON_INT;
  j->num_integer = value;
  return j;
}

Json *json_new_double(double value) {
  Json *j = json_new();
  j->type = JSON_DOUBLE;
  j->num_double = value;
  return j;
}

Json *json_new_bool(bool value) {
  Json *j = json_new();
  j->type = value ? JSON_TRUE : JSON_FALSE;
  return j;
}

Json *json_new_null() {
  Json *j = json_new();
  j->type = JSON_NULL;
  return j;
}

Json *json_new_object() {
  Json *j = json_new();
  j->object = json_object_new(JSON_OBJECT_SIZE_INIT);
  j->type = JSON_OBJECT;
  return j;
}

Json *json_new_array() {
  Json *j = json_new();
  j->array = json_array_new();
  j->type = JSON_ARRAY;
  return j;
}
