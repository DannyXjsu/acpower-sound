CC=gcc
CFLAGS=-O2
NAME=acpower-sound
BUILD_DIR=build

PREFIX_INSTALL=/opt

default:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR)
	$(CC) $(CFLAGS) $(NAME).c -o $(BUILD_DIR)/$(NAME)

.PHONY: install
install:
	@mkdir -p $(PREFIX_INSTALL)/$(NAME)
	@install -vm 755 $(BUILD_DIR)/$(NAME) $(PREFIX_INSTALL)/$(NAME)/$(NAME)

.PHONY: uninstall
uninstall:
	@rm -v $(PREFIX_INSTALL)/$(NAME)

.PHONY: clean
clean:
	@cd $(BUILD_DIR) && rm -v $(NAME)
