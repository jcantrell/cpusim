CC=c++
BINDIR := ../bin
BUILDDIR ?= ../build
DOCDIR ?= ../doc
INCDIR ?= .
#-Weffc++
CXXFLAGS=-I$(INCDIR) -Wunused -Wunreachable-code --coverage -std=c++11 \
-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused \
-fprofile-arcs -ftest-coverage
LDFLAGS=-lgcov --coverage
#find controller/ ui/ model/ -type f -name "*.cpp"
EX:=$(BUILDDIR)/controller/main.cpp.o
OBJ:=$(BUILDDIR)/ui/TextUI.cpp.o \
			$(BUILDDIR)/ui/parser/parser.cpp.o \
			$(BUILDDIR)/ui/parser/lexer.cpp.o \
			$(BUILDDIR)/ui/parser/abstracter.cpp.o \
			$(BUILDDIR)/model/cpu/cpu.cpp.o \
			$(BUILDDIR)/model/subleq/subleq.cpp.o \
			$(BUILDDIR)/model/mmix/mmix.cpp.o \
			$(BUILDDIR)/model/loader/Loader.cpp.o \
			$(BUILDDIR)/model/cpu/SignedMorsel.cpp.o \
			$(BUILDDIR)/model/cpu/UnsignedMorsel.cpp.o
TESTS:= $(BINDIR)/testUnsignedMorsel \
				$(BINDIR)/testSignedMorsel \
				$(BINDIR)/testcpu \
				$(BINDIR)/testLoader
  # BUILD_DIR ?= ./build
  # SRC_DIRS ?= ./src
  # 
  # SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
  # OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
  # DEPS := $(OBJS:.o=.d)

$(BINDIR)/cpusim: $(EX) $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) -o $(BINDIR)/cpusim $(EX) $(OBJ)

check: $(BINDIR)/cpusim $(TESTS)
	cppcheck . --enable=all

test: $(BINDIR)/cpusim $(TESTS) testUnsignedMorsel testSignedMorsel testCpu testLoader testSubleq testMmix coverageReport

testMmix: $(BINDIR)/cpusim $(TESTS)
	cd tests/functional/mmix && mmixal hello.mms && mmixal test.mms
	@echo
	cat tests/functional/mmix/hello.cps | $(BINDIR)/cpusim
	@echo
	cat tests/functional/mmix/test.cps | $(BINDIR)/cpusim
	@echo

$(BINDIR)/testcpu: $(BUILDDIR)/tests/unit/handrolled/testcpu.cpp.o $(BUILDDIR)/model/cpu/cpu.cpp.o $(BUILDDIR)/model/cpu/SignedMorsel.cpp.o $(BUILDDIR)/model/cpu/UnsignedMorsel.cpp.o
#$(BINDIR)/testcpu: $(OBJ) $(TESTS)
	$(CC) $(CXXFLAGS) -o $@ $?

$(BINDIR)/testSignedMorsel: $(BUILDDIR)/tests/unit/handrolled/testSignedMorsel.cpp.o  $(BUILDDIR)/model/cpu/SignedMorsel.cpp.o $(BUILDDIR)/model/cpu/UnsignedMorsel.cpp.o
#$(BINDIR)/testSignedMorsel: $(OBJ) $(TESTS)
	$(CC) $(CXXFLAGS) -o $@ $?

$(BINDIR)/testUnsignedMorsel: $(BUILDDIR)/tests/unit/handrolled/testUnsignedMorsel.cpp.o $(BUILDDIR)/model/cpu/UnsignedMorsel.cpp.o $(BUILDDIR)/model/cpu/SignedMorsel.cpp.o
#$(BINDIR)/testUnsignedMorsel: $(OBJ) $(TESTS)
	$(CC) $(CXXFLAGS) -o $@ $?

$(BINDIR)/testLoader: $(BUILDDIR)/tests/unit/handrolled/testLoader.cpp.o $(BUILDDIR)/model/loader/Loader.cpp.o $(BUILDDIR)/model/cpu/cpu.cpp.o $(BUILDDIR)/model/mmix/mmix.cpp.o $(BUILDDIR)/model/cpu/UnsignedMorsel.cpp.o $(BUILDDIR)/model/cpu/SignedMorsel.cpp.o
#$(BINDIR)/testLoader: $(OBJ) $(TESTS)
	$(CC) $(CXXFLAGS) -o $@ $?

$(BUILDDIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

coverageReport:
	lcov --capture --directory $(BUILDDIR) -b . --output-file $(BUILDDIR)/coverage.info
	lcov --remove $(BUILDDIR)/coverage.info -o $(BUILDDIR)/stripped_coverage.info "boost*" "c++*" "/usr/include/*"
	genhtml $(BUILDDIR)/stripped_coverage.info --output-dir $(DOCDIR)/reports

testSubleq: $(BINDIR)/cpusim $(TESTS)
	cat tests/functional/subleq/test_subleq | $(BINDIR)/cpusim
	@echo

testSignedMorsel: $(BINDIR)/cpusim $(TESTS)
	$(BINDIR)/testSignedMorsel
	@echo

testUnsignedMorsel: $(BINDIR)/cpusim $(TESTS)
	$(BINDIR)/testUnsignedMorsel
	@echo

testLoader: $(BINDIR)/cpusim $(TESTS)
	cd tests/functional/mmix && mmixal test.mms
	$(BINDIR)/testLoader
	@echo

testCpu: $(BINDIR)/cpusim $(TESTS)
	$(BINDIR)/testcpu
	@echo

prereqs:
	apt install libboost-all-dev lcov
	wget http://www.mmix.cs.hm.edu/bin/mmix -P ~/.bin
	wget http://www.mmix.cs.hm.edu/bin/mmixal -P ~/.bin
	wget http://www.mmix.cs.hm.edu/bin/mmmix -P ~/.bin
	wget http://www.mmix.cs.hm.edu/bin/mmotype -P ~/.bin
	chmod a+x ~/.bin/mmix
	chmod a+x ~/.bin/mmixal
	chmod a+x ~/.bin/mmmix
	chmod a+x ~/.bin/mmotype

clean:
	find . -type f \( -name "*.o" -o -name "*.mmo" -o -name "*.img" -o -name "*gcda" -o -name "*gcno" \) -delete
	find $(BINDIR) -name "*" -type f -executable -delete
	rm -rf $(TESTS) $(BUILDDIR)/* $(DOCDIR)/reports/*
