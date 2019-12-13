CC=c++
BINDIR := ../bin
BUILDDIR ?= ../build
INCDIR ?= .
#-Weffc++
CXXFLAGS=-I$(INCDIR) -Wunused -Wunreachable-code --coverage -std=c++11 \
-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused \
-fprofile-arcs -ftest-coverage
LDFLAGS=-lgcov --coverage
#find controller/ ui/ model/ -type f -name "*.cpp"
OBJ:=$(BUILDDIR)/controller/main.cpp.o \
			$(BUILDDIR)/ui/TextUI.cpp.o \
			$(BUILDDIR)/ui/parser/parser.cpp.o \
			$(BUILDDIR)/ui/parser/lexer.cpp.o \
			$(BUILDDIR)/ui/parser/abstracter.cpp.o \
			$(BUILDDIR)/model/cpu/cpu.cpp.o \
			$(BUILDDIR)/model/subleq/subleq.cpp.o \
			$(BUILDDIR)/model/mmix/mmix.cpp.o \
			$(BUILDDIR)/model/loader/Loader.cpp.o \
			$(BUILDDIR)/model/cpu/SignedMorsel.cpp.o \
			$(BUILDDIR)/model/cpu/UnsignedMorsel.cpp.o \
			$(BUILDDIR)/model/cpu/Address.cpp.o
TESTS:= $(BINDIR)/testmorsel \
				$(BINDIR)/testaddress \
				$(BINDIR)/testUnsignedMorsel \
				$(BINDIR)/testSignedMorsel \
				$(BINDIR)/testcpu \
				$(BINDIR)/testLoader
  # BUILD_DIR ?= ./build
  # SRC_DIRS ?= ./src
  # 
  # SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
  # OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
  # DEPS := $(OBJS:.o=.d)

$(BINDIR)/cpusim: $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) -o $(BINDIR)/cpusim $(OBJ)

check: $(BINDIR)/cpusim $(TESTS)
	cppcheck . --enable=all

test: $(BINDIR)/cpusim $(BINDIR)/testLoader $(TESTS)
	@echo 
	#$(BINDIR)/testmorsel && echo
	#$(BINDIR)/testLoader && echo
	#$(BINDIR)/testUnsignedMorsel
	@echo
	#$(BINDIR)/testSignedMorsel
	#@echo
	#$(BINDIR)/testaddress
	@echo
	#$(BINDIR)/testcpu
	@echo
	cd tests/functional && mmixal hello.mms && mmixal test.mms
	@echo
	#cat tests/functional/script | $(BINDIR)/cpusim
	cat tests/functional/script2 | $(BINDIR)/cpusim
	#cat tests/functional/subleqtest | $(BINDIR)/cpusim
	#lcov --capture --directory $(BUILDDIR) -b . --output-file $(BUILDDIR)/coverage.info
	#lcov --remove coverage.info '/usr/include/boost'
	#genhtml $(BUILDDIR)/coverage.info --output-dir ../doc/reports

#$(BINDIR)/testcpu: $(BUILDDIR)/tests/unit/handrolled/testcpu.cpp.o $(BUILDDIR)/model/cpu/Address.cpp.o $(BUILDDIR)/model/cpu/cpu.cpp.o
$(BINDIR)/testcpu: $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $?

#$(BINDIR)/testSignedMorsel: $(BUILDDIR)/tests/unit/handrolled/testSignedMorsel.cpp.o
$(BINDIR)/testSignedMorsel: $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $?

#$(BINDIR)/testUnsignedMorsel: $(BUILDDIR)/tests/unit/handrolled/testUnsignedMorsel.cpp.o
$(BINDIR)/testUnsignedMorsel: $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $?

#$(BINDIR)/testmorsel: $(BUILDDIR)/tests/unit/handrolled/testmorsel.cpp.o
$(BINDIR)/testmorsel: $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $?

#$(BINDIR)/testaddress: $(BUILDDIR)/tests/unit/handrolled/testaddress.cpp.o $(BUILDDIR)/model/cpu/Address.cpp.o
$(BINDIR)/testaddress: $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $?

#$(BINDIR)/testLoader: $(BUILDDIR)/tests/unit/handrolled/testLoader.cpp.o $(BUILDDIR)/model/cpu/Address.cpp.o $(BUILDDIR)/model/loader/Loader.cpp.o $(BUILDDIR)/model/cpu/cpu.cpp.o $(BUILDDIR)/model/mmix/mmix.cpp.o
$(BINDIR)/testLoader: $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $?

$(BUILDDIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	find . -type f \( -name "*.o" -o -name "*.mmo" -o -name "*.img" -o -name "*gcda" -o -name "*gcno" \) -delete
	find $(BINDIR) -name "*" -type f -executable -delete
	rm -rf $(TESTS) $(BUILDDIR)/*
