

SRC_DIR = $(shell pwd)
export BUILD_DIR = $(SRC_DIR)/build

.PHONY: pmparser
pmparser:
	mkdir -p $(BUILD_DIR) && \
	$(CC) $(SRC_DIR)/$@.c -c -o $(BUILD_DIR)/$@.o && \
	ar rcs $(BUILD_DIR)/lib$@.a $(BUILD_DIR)/$@.o


.PHONY: examples
examples: pmparser
	make -C $(SRC_DIR)/examples

