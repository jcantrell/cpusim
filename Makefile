CC=c++
CFLAGS=-I.

cpusim:	controller/main.o \
				ui/TextUI.o \
				ui/parser/parser.o \
				ui/parser/lexer.o \
				model/cpu.o \
				model/subleq.o 
	$(CC) $(CFLAGS) -o cpusim \
		controller/main.o \
		ui/TextUI.o \
		ui/parser/parser.o \
		ui/parser/lexer.o	\
		model/cpu.o \
		model/subleq.o

controller/main.o:
	$(CC) $(CFLAGS) -c -o controller/main.o controller/main.cpp

ui/TextUI.o:
	$(CC) $(CFLAGS) -c -o ui/TextUI.o ui/TextUI.cpp

ui/parser/parser.o:
	$(CC) $(CFLAGS) -c -o ui/parser/parser.o ui/parser/parser.cpp

ui/parser/lexer.o:
	$(CC) $(CFLAGS) -c -o ui/parser/lexer.o ui/parser/lexer.cpp

model/cpu.o:
	$(CC) $(CFLAGS) -c -o model/cpu.o model/cpu.cpp

model/subleq.o:
	$(CC) $(CFLAGS) -c -o model/subleq.o model/subleq.cpp


clean:
	rm cpusim.exe
	find . -name "*.o" -type f -delete
