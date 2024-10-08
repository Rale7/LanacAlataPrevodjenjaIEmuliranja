# komende za prevodjenje
CC = gcc
CXX = g++
CFLAGS = -g -pthread

# Tmp folder
TMP = ./tmp
TMP_OBJ = ./tmp/obj
TMP_DEP = ./tmp/dep
TMP_BISON = ./tmp/bison

BISON_FOLDER = ./misc

# Fajlovi potrebni za pravljenje asemblera
ASM_DIR = ./src/asembler
ASM_SRC_C = $(wildcard $(ASM_DIR)/*.c)
ASM_SRC_CPP = $(wildcard $(ASM_DIR)/*.cpp)

# Objektni_fajlovi za asembler
ASM_OBJ = $(patsubst $(ASM_DIR)/%.c, $(TMP_OBJ)/%_asm.o, $(ASM_SRC_C)) \
          $(patsubst $(ASM_DIR)/%.cpp, $(TMP_OBJ)/%_asm.o, $(ASM_SRC_CPP)) \
					$(TMP_OBJ)/bison_asm.o $(TMP_OBJ)/flex_asm.o

# Zavisni fajlovi za asembler
ASM_DEP = $(patsubst $(ASM_DIR)/%.c, $(TMP_DEP)/%_asm.d, $(ASM_SRC_C)) \
          $(patsubst $(ASM_DIR)/%.cpp, $(TMP_DEP)/%_asm.d, $(ASM_SRC_CPP)) \
					$(TMP_DEP)/bison_asm.d $(TMP_DEP)/flex_asm.d

# Asembler
ASM_PROGRAM = asembler

# Flex files
FLEX_FILE = misc/flex.l

# Generisani lekser
FLEX_FILES = $(TMP_OBJ)/lex.yy.o

# Napravi izvrsni fajl za asembler
$(ASM_PROGRAM): $(ASM_OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

# Prevedi izvorni C kod za asembler
$(TMP_OBJ)/%_asm.o: $(ASM_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

# Prevedi izvorni C++ kod za asembler
$(TMP_OBJ)/%_asm.o: $(ASM_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CFLAGS)

# Generisi parser
$(TMP_OBJ)/%_asm.o: $(BISON_FOLDER)/%.y
	@mkdir -p $(@D)
	@mkdir -p $(TMP_BISON)
	bison -d -o $(TMP_BISON)/bison.c $<
	$(CC) -c -o $@ $(TMP_BISON)/bison.c $(CFLAGS)

# Generisi lekser
$(TMP_OBJ)/%_asm.o: $(BISON_FOLDER)/%.l
	@mkdir -p $(@D)
	@mkdir -p $(TMP_BISON)
	flex -o $(TMP_BISON)/flex.c $<
	$(CC) -c -o $@ $(TMP_BISON)/flex.c $(CFLAGS)

# Napravi zavisne fajlove za C izvorne fajlove
$(TMP_DEP)/%_asm.d: $(ASM_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/$*_asm.o $< > $@

# Napravi zavisne fajlove za C++ izvorne fajlove
$(TMP_DEP)/%_asm.d: $(ASM_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -MM -MT $(TMP_OBJ)/$*_asm.o $< > $@

# Napravi zavisne fajlove za fajlove leksera i parsera
$(TMP_DEP)/%_asm.d: $(TMP_BISON)/%.c
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/$*_asm.o $< > $@

# Skripta za linker

# Linker
LD_PROGRAM=linker

LD_DIR=./src/linker
LD_SRC_C = $(wildcard $(LD_DIR)/*.c)
LD_SRC_CPP = $(wildcard $(LD_DIR)/*.cpp)

LD_OBJ = $(patsubst $(LD_DIR)/%.c, $(TMP_OBJ)/%_ld.o, $(LD_SRC_C)) \
				 $(patsubst $(LD_DIR)/%.cpp, $(TMP_OBJ)/%_ld.o, $(LD_SRC_CPP))

LD_DEP = $(patsubst $(LD_DIR)/%.c, $(TMP_DEP)/%_ld.d, $(LD_SRC_C)) \
				 $(patsubst $(LD_DIR)/%.cpp, $(TMP_DEP)/%_ld.d, $(LD_SRC_CPP))

$(LD_PROGRAM): $(LD_OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

$(TMP_OBJ)/%_ld.o: $(LD_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TMP_OBJ)/%_ld.o: $(LD_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CFLAGS)

$(TMP_DEP)/%_ld.d: $(LD_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/$*_ld.o $< > $@

$(TMP_DEP)/%_ld.d: $(LD_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/$*_ld.o $< > $@

#Emulator
EM_PROGRAM = emulator

EM_DIR=./src/emulator
EM_SRC_C = $(wildcard $(EM_DIR)/*.c)
EM_SRC_CPP = $(wildcard $(EM_DIR)/*.cpp) 

EM_OBJ =	$(patsubst $(EM_DIR)/%.c, $(TMP_OBJ)/%_em.o, $(EM_SRC_C)) \
					$(patsubst $(EM_DIR)/%.c, $(TMP_OBJ)/%.em_o, $(EM_SRC_CPP))

EM_DEP =	$(patsubst $(EM_DIR)/%.c, $(TMP_DEP)/%_em.d, $(EM_SRC_C)) \
					$(patsubst $(EM_DIR)/%.cpp, $(TMP_DEP)/%_em.d, $(EM_SRC_CPP))

$(EM_PROGRAM): $(EM_OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

$(TMP_OBJ)/%_em.o: $(EM_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TMP_OBJ)/%_em.o: $(EM_DIR)/%.cpp
	@mkdir -p $($D)
	$(CXX) -c -o $@ $< $(CFLAGS)

$(TMP_DEP)/%_em.d: $(EM_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/%*_em.o $< > $@

$(TMP_DEP)/%_ld.d: $(EM_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) -MM -MT $(TMP_OBJ)/$*_em.o $< > $@

# Ukljuci dependency fajlove
ifeq ($(MAKECMDGOALS), asembler)
-include $(ASM_DEP)
endif
ifeq ($(MAKECMDGOALS), linker)
-include $(LD_DEP)
endif

# Napravi sve fajlove

all: $(ASM_PROGRAM) $(LD_PROGRAM) $(EM_PROGRAM)

# Brisanje
clean:
	rm -r $(TMP)
	rm -f *~

.PHONY: clean
