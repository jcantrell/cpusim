cpusim: 
	c++ -I . -o cpusim \
		controller/main.cpp \
		ui/TextUI.cpp model/cpu.cpp \
		ui/parser/parser.cpp \
		ui/parser/lexer.cpp

clean:
	rm cpusim.exe
