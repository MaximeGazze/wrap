CC = gcc
TARGET = wrap
TEST_TARGET = test

.PHONY: build clean

build:
	$(CC) -o $(TARGET) $(TARGET).c

build_test:
	$(CC) -o $(TEST_TARGET) $(TEST_TARGET).c

clean:
	rm -f $(TARGET) $(TEST_TARGET)
