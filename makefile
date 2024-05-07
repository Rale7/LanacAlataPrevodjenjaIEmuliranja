# Command for compilation
CC = gcc
CXX = g++
CFLAGS = -g

# Temporary folder
TMP = ./tmp
TMP_OBJ = ./tmp/obj
TMP_DEP = ./tmp/dep
TMP_BISON = ./tmp/bison

BISON_FOLDER = ./misc

# Files needed for assembly
ASM_DIR = ./src/asembler
ASM_SRC_C = $(wildcard $(ASM_DIR)/*.c)
ASM_SRC_CPP = $(wildcard $(ASM_DIR)/*.cpp)

# Object files and dependency files
ASM_OBJ = $(patsubst $(ASM_DIR)/%.c, $(TMP_OBJ)/%_asm.o, $(ASM_SRC_C)) \
          $(patsubst $(ASM_DIR)/%.cpp, $(TMP_OBJ)/%_asm.o, $(ASM_SRC_CPP)) \
					$(TMP_OBJ)/bison_asm.o $(TMP_OBJ)/flex_asm.o

ASM_DEP = $(patsubst $(ASM_DIR)/%.c, $(TMP_DEP)/%_asm.d, $(ASM_SRC_C)) \
          $(patsubst $(ASM_DIR)/%.cpp, $(TMP_DEP)/%_asm.d, $(ASM_SRC_CPP)) \
					$(TMP_DEP)/bison_asm.d $(TMP_DEP)/flex_asm.d

# Program
ASM_PROGRAM = asembler

FLEX_FILE = misc/flex.l

FLEX_FILES = $(TMP_OBJ)/lex.yy.o

# Create the executable for the assembler
$(ASM_PROGRAM): $(ASM_OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

# Compile C source files to object files
$(TMP_OBJ)/%_asm.o: $(ASM_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

# Compile C++ source files to object files
$(TMP_OBJ)/%_asm.o: $(ASM_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CFLAGS)

$(TMP_OBJ)/%_asm.o: $(BISON_FOLDER)/%.y
	@mkdir -p $(@D)
	@mkdir -p $(TMP_BISON)
	bison -d -o $(TMP_BISON)/bison.c $<
	$(CC) -c -o $@ $(TMP_BISON)/bison.c $(CFLAGS)

$(TMP_OBJ)/%_asm.o: $(BISON_FOLDER)/%.l
	@mkdir -p $(@D)
	@mkdir -p $(TMP_BISON)
	flex -o $(TMP_BISON)/flex.c $<
	$(CC) -c -o $@ $(TMP_BISON)/flex.c $(CFLAGS)

# Create dependency files for C source files
$(TMP_DEP)/%_asm.d: $(ASM_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/$*_asm.o $< > $@

# Create dependency files for C++ source files
$(TMP_DEP)/%_asm.d: $(ASM_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -MM -MT $(TMP_OBJ)/$*_asm.o $< > $@

$(TMP_DEP)/%_asm.d: $(TMP_BISON)/%.c
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/$*_asm.o $< > $@

$(TMP_OBJ): $(TMP)
	mkdir $(TMP_OBJ)

$(TMP_DEP): $(TMP)
	mkdir $(TMP_DEP)

$(TMP_BISON): $(TMP)
	mkdir $(TMP_BISON)

$(TMP):
	mkdir $(TMP)

# Include dependency files
-include $(ASM_DEP)

# Clean rule
clean:
	rm -r $(TMP)
	rm -f *~

.PHONY: clean
