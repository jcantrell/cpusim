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
			$(BUILDDIR)/model/cpu/Address.cpp.o
TESTS:= $(BINDIR)/testmorsel \
				$(BINDIR)/testaddress \
				$(BINDIR)/testUnsignedMorsel \
				$(BINDIR)/testSignedMorsel \
				$(BINDIR)/testcpu

$(BINDIR)/cpusim: $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CXXFLAGS) -o $(BINDIR)/cpusim $(OBJ)

check: $(BINDIR)/cpusim $(TESTS)
	cppcheck . --enable=all

test: $(BINDIR)/cpusim $(TESTS)
	@echo 
	$(BINDIR)/testmorsel && echo
	$(BINDIR)/testUnsignedMorsel
	@echo
	$(BINDIR)/testSignedMorsel
	@echo
	$(BINDIR)/testaddress
	@echo
	$(BINDIR)/testcpu
	@echo
	cd tests/functional && mmixal hello.mms
	@echo
	cat tests/functional/script | $(BINDIR)/cpusim
	lcov --capture --directory ../build -b . --output-file coverage.info
	#lcov --remove coverage.info '/usr/include/boost'
	genhtml coverage.info --output-dir ../doc/reports

$(BINDIR)/testcpu: $(BUILDDIR)/tests/unit/handrolled/testcpu.cpp.o $(BUILDDIR)/model/cpu/Address.cpp.o $(BUILDDIR)/model/cpu/cpu.cpp.o
	$(CC) $(CXXFLAGS) -o $@ $?

$(BINDIR)/testSignedMorsel: $(BUILDDIR)/tests/unit/handrolled/testSignedMorsel.cpp.o
	$(CC) $(CXXFLAGS) -o $@ $?

$(BINDIR)/testUnsignedMorsel: $(BUILDDIR)/tests/unit/handrolled/testUnsignedMorsel.cpp.o
	$(CC) $(CXXFLAGS) -o $@ $?

$(BINDIR)/testmorsel: $(BUILDDIR)/tests/unit/handrolled/testmorsel.cpp.o
	$(CC) $(CXXFLAGS) -o $@ $?

$(BINDIR)/testaddress: $(BUILDDIR)/tests/unit/handrolled/testaddress.cpp.o $(BUILDDIR)/model/cpu/Address.cpp.o
	$(CC) $(CXXFLAGS) -o $@ $?

$(BUILDDIR)/%.cpp.o: %.cpp
	@echo THIS IS A THING
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo END THIS IS A THING

clean:
	find . -type f \( -name "*.o" -o -name "*.mmo" -o -name "*.img" -o -name "*gcda" -o -name "*gcno" \) -delete
	find $(BINDIR) -name "*" -type f -executable -delete
	rm -rf $(TESTS) $(BUILDDIR)/*
