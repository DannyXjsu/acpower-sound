CC=gcc
CFLAGS=-O2
NAME=acpower-sound
BUILD_DIR=build


default:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR)
	$(CC) $(CFLAGS) $(NAME).c -o $(BUILD_DIR)/$(NAME)

.PHONY: clean

clean:
	cd $(BUILD_DIR) && rm $(NAME)
