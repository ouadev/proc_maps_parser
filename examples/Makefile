


$(BUILD_DIR)/map: $(BUILD_DIR)/map.o
	$(CC) $^ -L$(BUILD_DIR) -lpmparser -o $@

$(BUILD_DIR)/map.o: map.c
	$(CC) $^ -I$(INCLUDE_DIR) -c -o $@


