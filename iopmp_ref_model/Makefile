# Directories
BIN_DIR  := bin
SRC_DIR  := src
VERIF    := verif
TEST_DIR := $(VERIF)/tests
LIB_DIR  := lib
cov = 
COVFLAGS = 
ifeq ($(cov), 1)
COVFLAGS += -fprofile-arcs -ftest-coverage -lgcov --coverage
endif

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Werror $(COVFLAGS) -I./include -Iverif/
LDFLAGS := -lm

# Common source files
COMMON_SOURCES := $(SRC_DIR)/iopmp_reg.c \
				  $(SRC_DIR)/iopmp_interrupt.c \
                  $(SRC_DIR)/iopmp_rule_analyzer.c \
                  $(SRC_DIR)/iopmp_validate.c \
                  $(SRC_DIR)/iopmp_error_capture.c \
                  $(VERIF)/test_utils.c

# Models and configurations
MODELS := full_model:fullmodel.c:0:0 \
          rapid_k_model:rapidmodel.c:0:1 \
          dynamic_k_model:dynamicmodel.c:0:2 \
          isolation_model:isolationmodel.c:1:0 \
          compact_k_model:compactmodel.c:1:1 \
          unnamed_model_1:unnamed_model_1.c:1:2 \
          unnamed_model_2:unnamed_model_2.c:2:0 \
          unnamed_model_3:unnamed_model_3.c:2:1 \
          unnamed_model_4:unnamed_model_4.c:2:2

# Targets
.PHONY: all build run clean

all: build run

# Ensure the directories exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Build all models into libraries and executables
build: $(BIN_DIR) $(LIB_DIR)
	@if [ -z "$(model)" ]; then \
		echo "Building all models..."; \
		for entry in $(MODELS); do \
			model=$$(echo $$entry | cut -d':' -f1); \
			source=$$(echo $$entry | cut -d':' -f2); \
			srcmd_fmt=$$(echo $$entry | cut -d':' -f3); \
			mdcfg_fmt=$$(echo $$entry | cut -d':' -f4); \
			echo "Building library and executable for $$model..."; \
			obj_files=""; \
			for src in $(COMMON_SOURCES) $(TEST_DIR)/$$source; do \
				obj_file=$$(basename $$src .c).o; \
				$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt -c $$src -o $$obj_file; \
				obj_files="$$obj_files $$obj_file"; \
			done; \
			ar rcD $(LIB_DIR)/$$model.a $$obj_files > /dev/null 2>&1; \
			ranlib $(LIB_DIR)/$$model.a; \
			$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt \
				$(TEST_DIR)/$$source -L$(LIB_DIR) -l:$$model.a -o $(BIN_DIR)/$$model $(LDFLAGS); \
			echo "Library and executable for $$model built successfully."; \
			rm -f $$obj_files; \
		done; \
	else \
		entry=$$(echo "$(MODELS)" | tr ' ' '\n' | grep -E "^$(model):"); \
		if [ -z "$$entry" ]; then \
			echo "Error: Model $(model) not found."; \
			exit 1; \
		fi; \
		source=$$(echo $$entry | cut -d':' -f2); \
		srcmd_fmt=$$(echo $$entry | cut -d':' -f3); \
		mdcfg_fmt=$$(echo $$entry | cut -d':' -f4); \
		echo "Building library and executable for $(model)..."; \
		obj_files=""; \
		for src in $(COMMON_SOURCES) $(TEST_DIR)/$$source; do \
			obj_file=$$(basename $$src .c).o; \
			$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt -c $$src -o $$obj_file; \
			obj_files="$$obj_files $$obj_file"; \
		done; \
		ar rcD $(LIB_DIR)/$(model).a $$obj_files > /dev/null 2>&1; \
		ranlib $(LIB_DIR)/$(model).a; \
		$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt \
				$(TEST_DIR)/$$source -L$(LIB_DIR) -l:$$model.a -o $(BIN_DIR)/$$model $(LDFLAGS); \
		echo "Library and executable for $(model) built successfully."; \
		rm -f $$obj_files; \
	fi

# Run all compiled binaries
run:
	@for model in $(MODELS); do \
		target=$${model%%:*}; \
		binary=$(BIN_DIR)/$$target; \
		if [ -f $$binary ]; then \
			echo "Running $$binary..."; \
			$$binary; \
		else \
			echo "Binary $$binary not found. Did you build it?"; \
		fi; \
	done

# Clean the binary and library directories
clean:
	rm -rf bin/ lib/ *.gcno *.gcda
