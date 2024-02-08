#include <assert.h>

#include "../src/json.h"

void test_lexer_string() {
  StringBuffer *input = sb_new_from_cstr("\"ABCDE\"");
  Lexer lexer = lexer_new(input);
  lexer_advance(&lexer);
  StringBuffer *string = lexer_read_string(&lexer);
  assert(string && "test_lexer_string string null");
  assert(string->len == 5 && "test_lexer_string string len");
  assert(sb_compare_sv(string, sv_new_from_cstr("ABCDE")) &&
         "test_lexer_string string equal");
}

void test_lexer() {
  test_lexer_string();
  printf("All 'lexer' tests passed successfully\n");
}
