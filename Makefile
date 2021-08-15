

SRC_DIR = $(shell pwd)
export BUILD_DIR ?= $(SRC_DIR)/build
export INCLUDE_DIR = $(SRC_DIR)/include

.PHONY: pmparser
pmparser:
	mkdir -p $(BUILD_DIR) && \
	$(CC) $(SRC_DIR)/$@.c -I$(INCLUDE_DIR) -c -o $(BUILD_DIR)/$@.o && \
	ar rcs $(BUILD_DIR)/lib$@.a $(BUILD_DIR)/$@.o


.PHONY: examples
examples: pmparser
	make -C $(SRC_DIR)/examples

