CC=c++
CFLAGS=-I.
OBJ:=controller/main.o \
			ui/TextUI.o \
			ui/parser/parser.o \
			ui/parser/lexer.o \
			model/cpu.o \
			model/subleq.o

cpusim:	$(OBJ)
	$(CC) $(CFLAGS) -o cpusim $(OBJ)

#$(OBJ):
#	$(CC) $(CFLAGS) -o $@

controller/main.o: controller/main.cpp ui/TextUI.h
	$(CC) $(CFLAGS) -c -o $@ controller/main.cpp

ui/TextUI.o: ui/TextUI.cpp ui/UI.h ui/TextUI.h
	$(CC) $(CFLAGS) -c -o $@ ui/TextUI.cpp

ui/parser/parser.o: ui/parser/parser.cpp ui/parser/parser.h
	$(CC) $(CFLAGS) -c -o $@ ui/parser/parser.cpp

ui/parser/lexer.o: ui/parser/lexer.cpp ui/parser/lexer.h
	$(CC) $(CFLAGS) -c -o $@ ui/parser/lexer.cpp

model/cpu.o: model/cpu.cpp model/cpu.h
	$(CC) $(CFLAGS) -c -o $@ model/cpu.cpp

model/subleq.o: model/subleq.cpp model/subleq.h
	$(CC) $(CFLAGS) -c -o $@ model/subleq.cpp

clean:
	rm cpusim.exe
	find . -name "*.o" -type f -delete
