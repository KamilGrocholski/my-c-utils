FLAGS=-Wall -Wextra -pedantic
SRC_FILES=src/string_utils.c src/uri.c src/logger.c src/json.c

TEST_BIN=test_bin
TEST_FLAGS=$(FLAGS) -g
TEST_SRC_FILES=$(SRC_FILES) test/main.c test/string_utils.c test/uri.c test/json.c
TEST_OUT_FILE=$(TEST_BIN)/main
 
.PHONY: test

test:
	rm -rf $(TEST_BIN)
	mkdir -p $(TEST_BIN)
	gcc $(TEST_FLAGS) $(TEST_SRC_FILES) -o ./$(TEST_OUT_FILE)
	./$(TEST_OUT_FILE) -h
