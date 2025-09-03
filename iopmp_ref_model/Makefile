# Directories
BIN_DIR  := bin
SRC_DIR  := src
VERIF    := verif
TEST_DIR := $(VERIF)/tests
LIB_DIR  := lib
LIBIOPMP_DIR := ../libiopmp
LIBIOPMP     := $(LIBIOPMP_DIR)/build/lib/libiopmp.a

cov =
COVFLAGS =
ifeq ($(cov), 1)
COVFLAGS += -fprofile-arcs -ftest-coverage -lgcov --coverage
endif

asan =
ASAN_CFLAGS =
ASAN_LDFLAGS =
ifeq ($(asan), 1)
ASAN_CFLAGS += -fsanitize=address -fno-omit-frame-pointer -O1 -g3
ASAN_LDFLAGS += -lasan
endif

libiopmp =
LIBIOPMP_CFLAGS =
LIBIOPMP_LDFLAGS =
LIBIOPMP_PREREQ =
ifeq ($(libiopmp), 1)
# Enable coverage analysis
cov = 1
LIBIOPMP_CFLAGS += -I$(LIBIOPMP_DIR)/include
LIBIOPMP_LDFLAGS += $(LIBIOPMP) -lgcov
# Prerequisite
LIBIOPMP_PREREQ += $(LIBIOPMP)
endif

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Werror $(COVFLAGS) $(ASAN_FLAGS) $(LIBIOPMP_CFLAGS) -I./include -Iverif/ -fPIC
LDFLAGS := -lm $(ASAN_LDFLAGS) $(LIBIOPMP_LDFLAGS)

# Common source files
COMMON_SOURCES := $(SRC_DIR)/iopmp_reg.c \
                  $(SRC_DIR)/iopmp_interrupt.c \
                  $(SRC_DIR)/iopmp_rule_analyzer.c \
                  $(SRC_DIR)/iopmp_validate.c \
                  $(SRC_DIR)/iopmp_error_capture.c \
                  $(VERIF)/test_utils.c

# Models and configurations
ifneq ($(libiopmp),1)
MODELS := full_model:fullmodel.c:0:0 \
          rapid_k_model:rapidmodel.c:0:1 \
          dynamic_k_model:dynamicmodel.c:0:2 \
          isolation_model:isolationmodel.c:1:0 \
          compact_k_model:compactmodel.c:1:1 \
          unnamed_model_1:unnamed_model_1.c:1:2 \
          unnamed_model_2:unnamed_model_2.c:2:0 \
          unnamed_model_3:unnamed_model_3.c:2:1 \
          unnamed_model_4:unnamed_model_4.c:2:2
else
MODELS := libiopmp_INFO:libiopmp_INFO.c:0:0
endif

# Targets
.PHONY: all build run clean

all: build run

# Ensure the directories exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# Build all models into libraries and executables
build: $(BIN_DIR) $(LIB_DIR) $(LIBIOPMP_PREREQ)
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
			if [ -n "$(sharedLib)" ]; then \
				$(CC) $$obj_files -shared -o $(LIB_DIR)/$$model.so; \
				$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt \
					$(TEST_DIR)/$$source -L$(LIB_DIR) -l:$$model.so -o $(BIN_DIR)/$$model $(LDFLAGS); \
			else \
				ar rcs $(LIB_DIR)/$$model.a $$obj_files > /dev/null 2>&1; \
				ranlib $(LIB_DIR)/$$model.a; \
				$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt \
					$(TEST_DIR)/$$source -L$(LIB_DIR) -l:$$model.a -o $(BIN_DIR)/$$model $(LDFLAGS); \
			fi; \
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
		if [ -n "$(sharedLib)" ]; then \
			$(CC) $$obj_files -shared -o $(LIB_DIR)/$$model.so; \
			$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt \
				$(TEST_DIR)/$$source -L$(LIB_DIR) -l:$$model.so -o $(BIN_DIR)/$$model $(LDFLAGS); \
		else \
			ar rcs $(LIB_DIR)/$$model.a $$obj_files > /dev/null 2>&1; \
			ranlib $(LIB_DIR)/$$model.a; \
			$(CC) $(CFLAGS) -DSRCMD_FMT=$$srcmd_fmt -DMDCFG_FMT=$$mdcfg_fmt \
				$(TEST_DIR)/$$source -L$(LIB_DIR) -l:$$model.a -o $(BIN_DIR)/$$model $(LDFLAGS); \
		fi; \
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

# Display usage information
.PHONY: help

help:
	@echo "Usage: make [TARGET] [VARIABLE=value]"
	@echo ""
	@echo "Targets:"
	@echo "  all           Build all models and run their binaries"
	@echo "  build         Compile all models or a specific model"
	@echo "                Use 'model=<model_name>' to specify a model"
	@echo "  run           Run all compiled binaries"
	@echo "                Ensure the models are built before running"
	@echo "  clean         Remove all compiled binaries, libraries, and coverage files"
	@echo "  help          Display this usage information"
	@echo ""
	@echo "Variables:"
	@echo "  model=<model_name>    Build a specific model instead of all models"
	@echo "                        Example: make build model=full_model"
	@echo "  sharedLib=1           Build shared libraries instead of static ones"
	@echo "  cov=1                 Enable code coverage during compilation"
	@echo "  asan=1                Enable AddressSanitizer"
	@echo "  libiopmp=1            Build libiopmp test program"
	@echo ""
	@echo "Available Models:"
	@echo "  full_model            SRCMD_FMT = 0, MDCFG_FMT = 0"
	@echo "  rapid_k_model         SRCMD_FMT = 0, MDCFG_FMT = 1"
	@echo "  dynamic_k_model       SRCMD_FMT = 0, MDCFG_FMT = 2"
	@echo "  isolation_model       SRCMD_FMT = 1, MDCFG_FMT = 0"
	@echo "  compact_k_model       SRCMD_FMT = 1, MDCFG_FMT = 1"
	@echo "  unnamed_model_1       SRCMD_FMT = 1, MDCFG_FMT = 2"
	@echo "  unnamed_model_2       SRCMD_FMT = 2, MDCFG_FMT = 0"
	@echo "  unnamed_model_3       SRCMD_FMT = 2, MDCFG_FMT = 1"
	@echo "  unnamed_model_4       SRCMD_FMT = 2, MDCFG_FMT = 2"
	@echo ""
	@echo "Available libiopmp test program:"
	@echo "  libiopmp_INFO         SRCMD_FMT = 0, MDCFG_FMT = 0"
	@echo ""

# Clean the binary and library directories
clean:
	rm -rf bin/ lib/ *.gcno *.gcda
