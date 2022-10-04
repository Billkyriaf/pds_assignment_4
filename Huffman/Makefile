CC = g++
CILK = /usr/local/OpenCilk-2.0.0/bin/clang++

BUILD_DIR := ./make-build-debug-g
CILK_BUILD_DIR := ./make-build-debug-cilk-20
PROF_DIR := ./profiling
SRC_DIRS := ./src
INC_DIRS := ./include

# Colors
GREEN = \033[1;32m
RED = \033[1;31m
NC = \033[0m
BOLD = \033[1m

# Directories
LIBRARIES_SRC := $(shell find $(INC_DIRS) -name '*.cpp')
LIBRARIES_SRC := $(LIBRARIES_SRC:%=$(BUILD_DIR)/%.o)

SERIAL_SRC := $(shell find $(SRC_DIRS)/serial -name '*.cpp')
SERIAL_SRC += $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp')
SERIAL_SRC := $(SERIAL_SRC:%=$(BUILD_DIR)/%.o)

PTHREAD_SRC := $(shell find $(SRC_DIRS)/pthread -name '*.cpp')
PTHREAD_SRC += $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp')
PTHREAD_SRC := $(PTHREAD_SRC:%=$(BUILD_DIR)/%.o)

CILK_SRC := $(shell find $(SRC_DIRS)/cilk -name '*.cpp')
CILK_SRC += $(shell find $(SRC_DIRS) -maxdepth 1 -name '*.cpp')
CILK_SRC := $(CILK_SRC:%=$(CILK_BUILD_DIR)/%.o)


# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d) $(shell find $(INC_DIRS) -type d)

# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC_FLAGS := $(INC_FLAGS) -O3 -Wall -g
PTHREAD_FLAGS := $(INC_FLAGS) -O3 -Wall -g -lpthread
CILK_FLAGS := $(INC_FLAGS) -O3 -Wall -g -fopencilk # -fsanitize=cilk -Og -g

all: build_serial build_pthread build_cilk


$(BUILD_DIR)/serial.out: $(SERIAL_SRC) $(LIBRARIES_SRC)
	@echo
	@echo -e "        $(BOLD)Linking...$(NC)"
	@echo
	@$(CC) $(CC_FLAGS) -o $(BUILD_DIR)/serial.out $(SERIAL_SRC) $(LIBRARIES_SRC)
	@echo -e "    $(GREEN)Build finished successfully!$(NC)"
	@echo


$(BUILD_DIR)/pthread.out: $(PTHREAD_SRC) $(LIBRARIES_SRC)
	@echo
	@echo -e "        $(BOLD)Linking...$(NC)"
	@echo
	@$(CC) $(PTHREAD_FLAGS) -o $(BUILD_DIR)/pthread.out $(PTHREAD_SRC) $(LIBRARIES_SRC)
	@echo -e "    $(GREEN)Build finished successfully!$(NC)"
	@echo


$(CILK_BUILD_DIR)/cilk.out: $(CILK_SRC) $(LIBRARIES_SRC)
	@echo
	@echo -e "        $(BOLD)Linking...$(NC)"
	@echo
	@$(CILK) $(CILK_FLAGS) -o $(CILK_BUILD_DIR)/cilk.out $(CILK_SRC) $(LIBRARIES_SRC)
	@echo -e "    $(GREEN)Build finished successfully!$(NC)"
	@echo


$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo -e "        $(BOLD)Compiling:$(NC) $(<)..."
	@$(CC) $(CC_FLAGS) -c $< -o $@


$(CILK_BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo -e "        $(BOLD)Compiling:$(NC) $(<)..."
	@$(CILK) $(CILK_FLAGS) -c $< -o $@

%.cpp:


build_serial: $(BUILD_DIR)/serial.out
run_serial: $(BUILD_DIR)/serial.out
	@echo
	@echo
	@$(BUILD_DIR)/serial.out
	@echo
	@echo


build_pthread: $(BUILD_DIR)/pthread.out
run_pthread: $(BUILD_DIR)/pthread.out
	@echo
	@echo
	@$(BUILD_DIR)/pthread.out
	@echo
	@echo


build_cilk: $(CILK_BUILD_DIR)/cilk.out
run_cilk: $(CILK_BUILD_DIR)/cilk.out
	@echo
	@echo
	@$(CILK_BUILD_DIR)/cilk.out
	@echo
	@echo


.PHONY: clean
clean:
	@echo -e "$(RED)Clearing build directories...$(NC)"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(CILK_BUILD_DIR)
	@echo -e "$(GREEN)Done!$(NC)"