CC=c++
#-Weffc++
CFLAGS=-Wunused -Wunreachable-code --coverage -std=c++11 -I. \
-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused \
-fprofile-arcs -ftest-coverage
LDFLAGS=-lgcov --coverage
OBJ:=controller/main.o \
			ui/TextUI.o \
			ui/parser/parser.o \
			ui/parser/lexer.o \
			model/cpu/cpu.o \
			model/subleq/subleq.o \
			model/mmix/mmix.o \
			model/cpu/Address.o
TESTS:=tests/unit/testmorsel \
				tests/unit/testaddress \
				tests/unit/testUnsignedMorsel \
				tests/unit/testSignedMorsel \
				tests/unit/testcpu

../bin/cpusim: cpusim

cpusim:	$(OBJ)
	mkdir -p ../bin
	$(CC) $(CFLAGS) -o ../bin/cpusim $(OBJ)

#$(OBJ):
#	$(CC) $(CFLAGS) -o $@
check: ../bin/cpusim $(TESTS)
	cppcheck . --enable=all

test: ../bin/cpusim $(TESTS)
	@echo
	./tests/unit/testmorsel
	@echo
	./tests/unit/testUnsignedMorsel
	@echo
	./tests/unit/testSignedMorsel
	@echo
	./tests/unit/testaddress
	@echo
	./tests/unit/testcpu
	@echo
	cd tests/functional && mmixal hello.mms
	@echo
	cat script | ../bin/cpusim

tests/unit/testcpu: tests/unit/handrolled/testcpu.cpp
	$(CC) $(CFLAGS) -o $@ tests/unit/handrolled/testcpu.cpp -I. model/cpu/Address.cpp model/cpu/cpu.cpp

tests/unit/testSignedMorsel.o: tests/unit/handrolled/testSignedMorsel.cpp
	$(CC) $(CFLAGS) -c -o $@ tests/unit/handrolled/testSignedMorsel.cpp

tests/unit/testUnsignedMorsel.o: tests/unit/handrolled/testUnsignedMorsel.cpp
	$(CC) $(CFLAGS) -c -o $@ tests/unit/handrolled/testUnsignedMorsel.cpp

tests/unit/testmorsel.o: tests/unit/handrolled/testmorsel.cpp
	$(CC) $(CFLAGS) -c -o $@ tests/unit/handrolled/testmorsel.cpp

tests/unit/testaddress: tests/unit/handrolled/testaddress.cpp model/cpu/Address.cpp
	$(CC) $(CFLAGS) -o $@ tests/unit/handrolled/testaddress.cpp model/cpu/Address.cpp

controller/main.o: controller/main.cpp ui/TextUI.h
	$(CC) $(CFLAGS) -c -o $@ controller/main.cpp

ui/TextUI.o: ui/TextUI.cpp ui/UI.h ui/TextUI.h
	$(CC) $(CFLAGS) -c -o $@ ui/TextUI.cpp

ui/parser/parser.o: ui/parser/parser.cpp ui/parser/parser.h
	$(CC) $(CFLAGS) -c -o $@ ui/parser/parser.cpp

ui/parser/lexer.o: ui/parser/lexer.cpp ui/parser/lexer.h
	$(CC) $(CFLAGS) -c -o $@ ui/parser/lexer.cpp

model/cpu/cpu.o: model/cpu/cpu.cpp model/cpu/cpu.h model/cpu/Address.cpp
	$(CC) $(CFLAGS) -c -o $@ model/cpu/cpu.cpp

model/subleq/subleq.o: model/subleq/subleq.cpp model/subleq/subleq.h
	$(CC) $(CFLAGS) -c -o $@ model/subleq/subleq.cpp

model/mmix/mmix.o: model/mmix/mmix.cpp model/mmix/mmix.h
	$(CC) $(CFLAGS) -c -o $@ model/mmix/mmix.cpp

model/mmix/Address.o: model/cpu/Address.cpp
	$(CC) $(CFLAGS) -c -o $@ model/cpu/Address.cpp

clean:
	find . -name "*.o" -type f -delete
	find . -name "*.exe" -type f -delete
	find . -name "*.mmo" -type f -delete
	find . -name "*.img" -type f -delete
	find ../bin -name "*" -type f -delete
	[ ! -f tests/unit/testcpu ] || rm tests/unit/testcpu
	[ ! -f tests/unit/testaddress ] || rm tests/unit/testaddress
	[ ! -e tests/unit/testmorsel ] || rm tests/unit/testmorsel
	[ ! -e tests/unit/testUnsignedMorsel ] || rm tests/unit/testUnsignedMorsel
	[ ! -e tests/unit/testSignedMorsel ] || rm tests/unit/testSignedMorsel
