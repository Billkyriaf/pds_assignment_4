CC = g++
BUILD_DIR := ./make_build
PROF_DIR := ./profiling
SRC_DIRS := ./src
INC_DIRS := ./include

# Colors
GREEN = \033[1;32m
RED = \033[1;31m
NC = \033[0m

# Directories
LIBRARIES_SRC := $(shell find $(INC_DIRS) -name '*.cpp')
LIBRARIES_SRC := $(LIBRARIES_SRC:%=$(BUILD_DIR)/%.o)

SERIAL_SRC := $(shell find $(SRC_DIRS)/serial -name '*.cpp')
SERIAL_SRC := $(SERIAL_SRC:%=$(BUILD_DIR)/%.o)

PTHREAD_SRC := $(shell find $(SRC_DIRS)/pthread -name '*.cpp')
PTHREAD_SRC := $(PTHREAD_SRC:%=$(BUILD_DIR)/%.o)


# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d) $(shell find $(INC_DIRS) -type d)

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS := $(INC_FLAGS) -O3 -Wall -g # -pg

all: pre_build build_serial build_pthread post_build


pre_build:
	@echo -e "$(GREEN)Building all...$(NC)"


post_build:
	@echo -e "$(GREEN)Build all successful!!$(NC)"


build_serial: $(SERIAL_SRC) $(LIBRARIES_SRC)
	@echo -e "    $(GREEN)Building Serial binary...$(NC)"
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/serial.out $(SERIAL_SRC) $(LIBRARIES_SRC)
	@echo -e "    $(GREEN)Build finished successfully!$(NC)"
	@echo


build_pthread: $(PTHREAD_SRC) $(LIBRARIES_SRC)
	@echo -e "    $(GREEN)Building pthread binary...$(NC)"
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/pthread.out $(PTHREAD_SRC) $(LIBRARIES_SRC) -lpthread
	@echo -e "    $(GREEN)Build finished successfully!$(NC)"
	@echo


$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@


run_serial: build_serial
	@echo -e "$(GREEN)Running serial...$(NC)"
	@$(BUILD_DIR)/serial.out

run_pthread: build_pthread
	@echo -e "$(GREEN)Running pthread...$(NC)"
	@$(BUILD_DIR)/pthread.out

.PHONY: clean
clean:
	@echo -e "$(RED)Clearing build directory...$(NC)"
	@rm -rf $(BUILD_DIR)
	@echo -e "$(GREEN)Done!$(NC)"