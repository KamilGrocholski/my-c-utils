#include <ctype.h>
#include <stdlib.h>

#include "json.h"
#include "logger.h"

Lexer lexer_new(StringView *input) {
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
    /* if (lexer->ch == '\n') { */
    /*   lexer->location.line++; */
    /*   lexer->location.offset = 0; */
    /* } else { */
    /*   lexer->location.offset++; */
    /* } */
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

void lexer_eat(Lexer *lexer, char ch) {
  if (lexer->ch != ch) {
    printf("expected: '%c', got: '%c'\n", ch, lexer->ch);
    logger_log(LOG_ERROR, "lexer_eat error");
    exit(1);
  }
  lexer_advance(lexer);
}

StringView lexer_read_integer(Lexer *lexer) {
  size_t start = lexer->idx;
  while (isdigit(lexer->ch)) {
    lexer_advance(lexer);
  }
  size_t end = lexer->idx;
  return sv_sub(*lexer->input, start, end - start);
}

StringView lexer_read_ident(Lexer *lexer) {
  size_t start = lexer->idx;
  while (isalpha(lexer->ch)) {
    lexer_advance(lexer);
  }
  size_t end = lexer->idx;
  return sv_sub(*lexer->input, start, end - start);
}

StringView lexer_read_string(Lexer *lexer) {
  lexer_eat(lexer, '"');
  size_t start = lexer->idx;
  while (lexer->ch != '"' && lexer->ch != JSON_END_OF_INPUT) {
    lexer_advance(lexer);
  }
  size_t end = lexer->idx;
  lexer_eat(lexer, '"');
  return sv_sub(*lexer->input, start, end - start);
}

Json *json_new() {
  Json *json = malloc(sizeof(Json));
  if (json == NULL) {
    logger_log(LOG_ERROR, "json_new mem alloc");
    exit(1);
  }
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
  case JSON_INT:
  case JSON_DOUBLE:
  case JSON_TRUE:
  case JSON_FALSE:
  case JSON_NULL:
    free(json);
    break;
  default:
    logger_log(LOG_ERROR, "json_free invalid json type");
    exit(1);
  }
}

Json *json_parse(StringView *input) {
  Lexer lexer = lexer_new(input);
  lexer_advance(&lexer);

  return json_parse_value(&lexer);
}

Json *json_parse_value(Lexer *lexer) {
  lexer_skip_whitespace(lexer);
  switch (lexer->ch) {
  case '"':
    return json_parse_string(lexer);
  case '[':
    return json_parse_array(lexer);
  case '{':
    return json_parse_object(lexer);
  default: {
    if (isdigit(lexer->ch) || lexer->ch == '-') {
      Json *value = json_new();
      size_t start = lexer->idx;
      lexer_advance(lexer);
      while (isdigit(lexer->ch)) {
        lexer_advance(lexer);
      }
      if (lexer->ch == '.') {
        lexer_eat(lexer, '.');
        while (isdigit(lexer->ch)) {
          lexer_advance(lexer);
        }
        value->type = JSON_DOUBLE;
        value->num_double =
            atof(sv_sub(*lexer->input, start, lexer->idx - start).data);
      } else {
        value->type = JSON_INT;
        value->num_integer =
            atoi(sv_sub(*lexer->input, start, lexer->idx - start).data);
      }
      return value;
    } else if (isalpha(lexer->ch)) {
      StringView ident = lexer_read_ident(lexer);
      Json *value = json_new();
      if (sv_compare(ident, sv_new("null", 4))) {
        value->type = JSON_NULL;
      } else if (sv_compare(ident, sv_new("true", 4))) {
        value->type = JSON_TRUE;
      } else if (sv_compare(ident, sv_new("false", 5))) {
        value->type = JSON_FALSE;
      } else {
        logger_log(LOG_ERROR, "invalid keyword: '%.*s'", (int)ident.len,
                   ident.data);
        exit(1);
      }
      return value;
    }
  }
  }
  return NULL;
}

Json *json_parse_string(Lexer *lexer) {
  StringView string = lexer_read_string(lexer);
  Json *json = json_new();
  *json = (Json){
      .type = JSON_STRING,
      .string = string,
  };
  return json;
}

Json *json_parse_array(Lexer *lexer) {
  Json *json = json_new();
  JsonArray *array = json_array_new();
  *json = (Json){
      .type = JSON_ARRAY,
      .array = array,
  };
  lexer_eat(lexer, '[');
  lexer_skip_whitespace(lexer);
  if (lexer->ch == ']') {
    return json;
  }

  lexer_skip_whitespace(lexer);

  json_array_append(array, json_parse_value(lexer));
  lexer_skip_whitespace(lexer);

  while (lexer->ch == ',') {
    lexer_eat(lexer, ',');
    json_array_append(array, json_parse_value(lexer));
    lexer_skip_whitespace(lexer);
  }

  lexer_skip_whitespace(lexer);
  lexer_eat(lexer, ']');

  return json;
}

void json_array_free(JsonArray *a) {
  for (size_t i = 0; i < a->len; ++i) {
    json_free(a->items[i]);
  }
  free(a->items);
  free(a);
}

Json *json_parse_object(Lexer *lexer) {
  Json *json = json_new();
  JsonObject *object = json_object_new(JSON_OBJECT_SIZE_INIT);
  *json = (Json){
      .type = JSON_OBJECT,
      .object = object,
  };

  lexer_eat(lexer, '{');
  lexer_skip_whitespace(lexer);
  if (lexer->ch == '}') {
    return json;
  }

  lexer_skip_whitespace(lexer);
  StringView key = lexer_read_string(lexer);
  lexer_skip_whitespace(lexer);
  lexer_eat(lexer, ':');
  Json *value = json_parse_value(lexer);
  json_object_set(object, key, value);
  lexer_skip_whitespace(lexer);

  while (lexer->ch == ',') {
    lexer_eat(lexer, ',');
    lexer_skip_whitespace(lexer);
    key = lexer_read_string(lexer);
    lexer_skip_whitespace(lexer);
    lexer_eat(lexer, ':');
    value = json_parse_value(lexer);
    json_object_set(object, key, value);
  }

  lexer_skip_whitespace(lexer);
  lexer_eat(lexer, '}');

  return json;
}

JsonArray *json_array_new() {
  JsonArray *a = malloc(sizeof(JsonArray));
  if (a == NULL) {
    logger_log(LOG_ERROR, "json_array_new mem alloc err");
    exit(1);
  }
  *a = (JsonArray){
      .len = 0,
      .cap = JSON_ARRAY_CAP_INIT,
      .items = malloc(sizeof(Json *) * JSON_ARRAY_CAP_INIT),
  };
  if (a->items == NULL) {
    logger_log(LOG_ERROR, "json_array_new->items mem alloc err");
    exit(1);
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
    logger_log(LOG_ERROR, "json_array_resize->items mem realloc err");
    exit(1);
  }
  a->cap = new_cap;
}

JsonObject *json_object_new(size_t size) {
  JsonObject *o = malloc(sizeof(JsonObject));
  if (o == NULL) {
    logger_log(LOG_ERROR, "json_object_new mem alloc err");
    exit(1);
  }
  *o = (JsonObject){
      .size = size,
      .buckets = calloc(size, sizeof(JsonObjectPair *)),
  };
  if (o->buckets == NULL) {
    logger_log(LOG_ERROR, "json_object_new->buckets mem alloc err");
    exit(1);
  }
  return o;
}

void json_object_set(JsonObject *o, StringView key, Json *value) {
  size_t hash = json_object_hash(key);
  size_t idx = hash % o->size;

  JsonObjectPair *new_pair = malloc(sizeof(JsonObjectPair));
  if (new_pair == NULL) {
    logger_log(LOG_ERROR, "json_object_set mem alloc err");
    exit(1);
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
    if (sv_compare(curr->key, key)) {
      return curr->value;
    }
    curr = curr->next;
  }
  return NULL;
}

size_t json_object_hash(StringView key) {
  size_t h = 0;
  while (key.len) {
    h = 31 * h + *(key.data)++;
    --key.len;
  }
  return h;
}

void json_object_foreach(JsonObject *o,
                         void (*callback)(StringView key, Json *value)) {
  for (size_t i = 0; i < o->size; ++i) {
    JsonObjectPair *pair = o->buckets[i];
    while (pair) {
      callback(pair->key, pair->value);
      pair = pair->next;
    }
  }
}

void json_object_free(JsonObject *o) {
  for (size_t i = 0; i < o->size; ++i) {
    JsonObjectPair *curr = o->buckets[i];
    while (curr != NULL) {
      JsonObjectPair *next = curr->next;
      json_free(curr->value);
      free(curr);
      curr = next;
    }
  }
  free(o->buckets);
  free(o);
}

void _json_obj_print_cb(StringView key, Json *value) {
  printf(", ");
  printf("\"%.*s\": ", (int)key.len, key.data);
  json_print(value);
}

void json_print(Json *json) {
  switch (json->type) {
  case JSON_STRING:
    printf("\'%.*s\'", (int)json->string.len, json->string.data);
    break;
  case JSON_ARRAY:
    printf("[");
    for (size_t i = 0; i < json->array->len; ++i) {
      if (i > 0)
        printf(", ");
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
  }
}
