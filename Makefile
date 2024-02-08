FLAGS=-Wall -Wextra -pedantic
DEBUG_FLAGS=-g $(FLAGS)
SRC_FILES=src/string_utils.c src/uri.c src/logger.c src/json.c

TEST_BIN=test_bin
TEST_SRC_FILES=$(SRC_FILES) test/main.c test/string_utils.c test/uri.c test/json.c test/lexer.c
TEST_OUT_FILE=$(TEST_BIN)/main

.PHONY: test debug

test:
	rm -rf $(TEST_BIN)
	mkdir -p $(TEST_BIN)
	gcc $(FLAGS) -o ./$(TEST_OUT_FILE) $(TEST_SRC_FILES)
	./$(TEST_OUT_FILE) -h

debug:
	rm -rf $(TEST_BIN)
	mkdir -p $(TEST_BIN)
	gcc $(DEBUG_FLAGS) -o ./$(TEST_OUT_FILE) $(TEST_SRC_FILES)
	gdb ./$(TEST_OUT_FILE)
