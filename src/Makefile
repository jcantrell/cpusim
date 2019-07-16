CC=c++
CFLAGS=-std=c++11 -I.
OBJ:=controller/main.o \
			ui/TextUI.o \
			ui/parser/parser.o \
			ui/parser/lexer.o \
			model/cpu/cpu.o \
			model/subleq/subleq.o \
			model/mmix/mmix.o \
			model/cpu/Address.o

cpusim:	$(OBJ)
	mkdir -p ../bin
	$(CC) $(CFLAGS) -o ../bin/cpusim $(OBJ)

#$(OBJ):
#	$(CC) $(CFLAGS) -o $@

test: ../bin/cpusim tests/unit/testmorsel
	./tests/unit/testmorsel
	cd tests && mmixal hello.mms
	cat script | ../bin/cpusim

tests/unit/testmorsel.o: tests/unit/testmorsel.cpp
	$(CC) $(CFLAGS) -c -o $@ tests/unit/testmorsel.cpp

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

clean:
	find . -name "*.o" -type f -delete
	find . -name "*.exe" -type f -delete
	find . -name "*.mmo" -type f -delete
	find . -name "*.img" -type f -delete
