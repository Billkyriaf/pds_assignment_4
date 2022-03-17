CC = gcc
CILKCC = /usr/local/OpenCilk-9.0.1-Linux/bin/clang


BUILD_DIR := ./build
SRC_DIRS := ./src

# Colors
# GREEN = \033[1;32m
# RED = \033[1;31m
# NC = \033[0m

GREEN =
RED =
NC =

# Directories
LIBRARIES_SRC := $(shell find $(SRC_DIRS)/ -name '*utils.c')
LIBRARIES_SRC := $(LIBRARIES_SRC:%=$(BUILD_DIR)/%.o)

SEQUENTIAL_SRC := $(SRC_DIRS)/axpy_sequential.c
SEQUENTIAL_SRC := $(SEQUENTIAL_SRC:%=$(BUILD_DIR)/%.o)

PTHREAD_SRC := $(SRC_DIRS)/axpy_pthread.c
PTHREAD_SRC := $(PTHREAD_SRC:%=$(BUILD_DIR)/%.o)

OPENCILK_SRC := $(SRC_DIRS)/axpy_cilkfor.c
OPENCILK_SRC := $(OPENCILK_SRC:%=$(BUILD_DIR)/%.o)


# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS := $(INC_FLAGS) -O3 -w


# Isolate the trget name and pass everything else as argument
RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
# ...and turn them into do-nothing targets
$(eval $(RUN_ARGS):;@:)


all: clean pre_build build_sequential build_pthread build_cilkfor post_build


pre_build:
	@echo "$(GREEN)Building all...$(NC)"


post_build:
	@echo "$(GREEN)Build all successful!!$(NC)"


build_sequential: $(SEQUENTIAL_SRC) $(LIBRARIES_SRC)
	@echo "    $(GREEN)Building Serial binary...$(NC)"
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/sequential.out $(SEQUENTIAL_SRC) $(LIBRARIES_SRC)
	@echo "    $(GREEN)Build finished successfully!$(NC)"
	@echo


build_pthread: $(PTHREAD_SRC) $(LIBRARIES_SRC)
	@echo "    $(GREEN)Building pthread binary...$(NC)"
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/pthread.out $(PTHREAD_SRC) $(LIBRARIES_SRC) -lpthread
	@echo "    $(GREEN)Build finished successfully!$(NC)"
	@echo


build_cilkfor: $(OPENCILK_SRC) $(LIBRARIES_SRC)
	@echo "    $(GREEN)Building OpenCilk binary...$(NC)"
	@$(CILKCC) $(CFLAGS) -o $(BUILD_DIR)/cilkfor.out $(OPENCILK_SRC) $(LIBRARIES_SRC) -fopencilk
	@echo "    $(GREEN)Build finished successfully!$(NC)"
	@echo


$(BUILD_DIR)/$(SRC_DIRS)/axpy_cilkfor.c.o: $(SRC_DIRS)/axpy_cilkfor.c
	@mkdir -p $(dir $@)
	@$(CILKCC) $(CFLAGS) -c $< -o $@ -fopencilk


$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@


run_seq: build_sequential
	@./$(BUILD_DIR)/sequential.out $(RUN_ARGS)

run_pthread: build_pthread
	@./$(BUILD_DIR)/pthread.out $(RUN_ARGS)

run_cilkfor: build_cilkfor
	@./$(BUILD_DIR)/cilkfor.out $(RUN_ARGS)

.PHONY: clean
clean:
	@echo "$(RED)Clearing build directory...$(NC)"
	@rm -rf $(BUILD_DIR)
	@echo "$(GREEN)Done!$(NC)"